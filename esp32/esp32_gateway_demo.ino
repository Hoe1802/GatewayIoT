#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ====== PIN ======
#define PIN_DHT   4
#define DHTTYPE   DHT11
#define PIN_LED   2
#define PIN_BTN   27   // tactile -> GND, INPUT_PULLUP

// ====== WIFI (2.4GHz) ======
const char* WIFI_SSID = "Hoa Hien";
const char* WIFI_PASS = "tuikhongnhomatkhau@6";

// ====== MQTT ======
const char* MQTT_HOST = "192.168.1.8";
const int   MQTT_PORT = 1883;

// ====== TOPIC ======
const char* TOPIC_DHT        = "gateway/demo/dht";
const char* TOPIC_LED_SET    = "gateway/demo/led/set";
const char* TOPIC_LED_STATE  = "gateway/demo/led/state";
const char* TOPIC_BTN        = "gateway/demo/switch";
const char* TOPIC_STATUS     = "gateway/demo/status"; // ONLINE/OFFLINE (LWT)

WiFiClient espClient;
PubSubClient mqtt(espClient);
DHT dht(PIN_DHT, DHTTYPE);

// ====== STATE ======
volatile bool ledStateISR = false;     // LED state updated in ISR
volatile bool btnPressedEvent = false; // event flag set in ISR
volatile unsigned long lastIrqMs = 0;  // debounce timestamp

bool ledState = false;                // mirror in main loop

// Timers
unsigned long lastWifiAttemptMs = 0;
unsigned long lastMqttAttemptMs = 0;
unsigned long lastDhtMs = 0;

const unsigned long wifiRetryMs = 5000;
const unsigned long mqttRetryMs = 3000;
const unsigned long dhtIntervalMs = 2000;

wl_status_t lastWifiStatus = WL_IDLE_STATUS;

// ====== ISR: nút bấm toggle LED NGAY LẬP TỨC ======
void IRAM_ATTR onButtonFalling() {
  unsigned long now = millis();
  if (now - lastIrqMs < 200) return;  // debounce 200ms
  lastIrqMs = now;

  ledStateISR = !ledStateISR;
  digitalWrite(PIN_LED, ledStateISR ? HIGH : LOW); // update LED immediately
  btnPressedEvent = true;                           // notify main loop
}

// ====== Helpers ======
void publishLedState() {
  if (mqtt.connected()) {
    mqtt.publish(TOPIC_LED_STATE, ledState ? "ON" : "OFF", true);
  }
}

void setLedMain(bool on, bool publishState = true) {
  ledState = on;
  ledStateISR = on;
  digitalWrite(PIN_LED, on ? HIGH : LOW);

  Serial.print("[LED] ");
  Serial.println(on ? "ON" : "OFF");

  if (publishState) publishLedState();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  Serial.print("[MQTT] ");
  Serial.print(topic);
  Serial.print(" => ");
  Serial.println(msg);

  if (String(topic) == TOPIC_LED_SET) {
    if (msg.equalsIgnoreCase("ON") || msg == "1" || msg.equalsIgnoreCase("TRUE")) {
      setLedMain(true, true);
    } else if (msg.equalsIgnoreCase("OFF") || msg == "0" || msg.equalsIgnoreCase("FALSE")) {
      setLedMain(false, true);
    }
  }
}

// ====== WIFI: start + retry (không block) ======
void startWiFi() {
  Serial.println("[WIFI] Starting...");
  Serial.print("[WIFI] SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void tryWiFi() {
  wl_status_t st = WiFi.status();

  // log when state changes
  if (st != lastWifiStatus) {
    lastWifiStatus = st;
    Serial.print("[WIFI] Status=");
    Serial.println((int)st);

    if (st == WL_CONNECTED) {
      Serial.print("[WIFI] Connected. IP=");
      Serial.println(WiFi.localIP());
      Serial.print("[WIFI] RSSI=");
      Serial.println(WiFi.RSSI());
    }
  }

  if (st == WL_CONNECTED) return;
  if (millis() - lastWifiAttemptMs < wifiRetryMs) return;

  lastWifiAttemptMs = millis();
  Serial.println("[WIFI] Reconnecting...");
  WiFi.disconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

// ====== MQTT: retry (giảm block connect) ======
void tryMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) return;
  if (millis() - lastMqttAttemptMs < mqttRetryMs) return;

  lastMqttAttemptMs = millis();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(1);  // quan trọng: giảm thời gian chờ để không “cảm giác treo”

  String cid = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("[MQTT] Connecting ");
  Serial.print(MQTT_HOST);
  Serial.print(":");
  Serial.println(MQTT_PORT);

  bool ok = mqtt.connect(
    cid.c_str(),
    TOPIC_STATUS, 1, true, "OFFLINE" // LWT
  );

  if (ok) {
    Serial.println("[MQTT] Connected");
    mqtt.subscribe(TOPIC_LED_SET);

    mqtt.publish(TOPIC_STATUS, "ONLINE", true);
    publishLedState(); // sync LED state khi vừa connect
  } else {
    Serial.print("[MQTT] FAIL rc=");
    Serial.println(mqtt.state()); // -2 nếu broker tắt/không tới được
  }
}

// ====== DHT publish (chỉ khi MQTT connected) ======
void publishDHT() {
  if (!mqtt.connected()) return;
  if (millis() - lastDhtMs < dhtIntervalMs) return;
  lastDhtMs = millis();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    char payload[80];
    snprintf(payload, sizeof(payload), "{\"temp\":%.1f,\"hum\":%.1f}", t, h);
    mqtt.publish(TOPIC_DHT, payload, false);

    Serial.print("[DHT] ");
    Serial.println(payload);
  } else {
    Serial.println("[DHT] Read error");
  }
}

void setup() {
  Serial.begin(115200);
  delay(600);
  Serial.println("===== ESP32 START =====");

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  // LED off
  setLedMain(false, false);

  // Attach interrupt for button (instant toggle)
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), onButtonFalling, FALLING);

  dht.begin();
  startWiFi();
}

void loop() {
  // xử lý event nút bấm (LED đã toggle ngay trong ISR rồi)
  if (btnPressedEvent) {
    btnPressedEvent = false;

    // sync main state with ISR state
    ledState = ledStateISR;

    Serial.print("[BTN] Pressed -> LED ");
    Serial.println(ledState ? "ON" : "OFF");

    // publish event/state nếu MQTT đang connected
    if (mqtt.connected()) {
      mqtt.publish(TOPIC_BTN, "PRESSED", false);
      publishLedState();
    }
  }

  // retry kết nối WiFi/MQTT ở nền
  tryWiFi();
  tryMQTT();

  // xử lý mqtt loop + publish DHT
  if (mqtt.connected()) mqtt.loop();
  publishDHT();
}
