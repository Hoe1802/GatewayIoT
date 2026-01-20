# Gateway IoT trên nền tảng AOSP Android TV cho Raspberry Pi 4

## 1. Giới thiệu

Đồ án xây dựng một **Gateway IoT** chạy trên **Raspberry Pi 4**, sử dụng **AOSP Android TV (Android 14)** làm hệ điều hành nền tảng.

Gateway đóng vai trò trung gian giữa **thiết bị IoT (ESP32 + cảm biến)** và **người dùng**, thực hiện các chức năng:

* Thu thập dữ liệu cảm biến từ thiết bị IoT qua giao thức **MQTT**
* Xử lý dữ liệu tại biên (Edge Computing) ngay trên Gateway
* Hiển thị dữ liệu thời gian thực qua Dashboard
* Điều khiển thiết bị IoT từ xa
* Tích hợp **Node-RED** để xử lý luồng dữ liệu
* Sử dụng **Mosquitto MQTT Broker** cho cơ chế publish/subscribe

Toàn bộ hệ thống được triển khai và kiểm thử trên **Raspberry Pi 4**.

---

## 2. Kiến trúc tổng thể hệ thống

Hệ thống bao gồm các thành phần chính:

* **ESP32**: đọc DHT11 + nút nhấn, điều khiển LED; publish/subscribe qua MQTT
* **Gateway (Raspberry Pi 4 – AOSP Android TV)**:

  * Mosquitto MQTT Broker
  * Node-RED + Dashboard
* **Người dùng**: theo dõi dữ liệu và điều khiển thiết bị qua Dashboard

---

## 3. Cấu trúc thư mục dự án

```text
GatewayIoT/
├── manifest/        # Manifest AOSP lock phiên bản
├── patches/         # Patch thay đổi AOSP cho Raspberry Pi 4
├── scripts/         # Script init source, apply patch, build AOSP
├── docs/            # Tài liệu môi trường và hướng dẫn chi tiết
├── esp32/           # Mã nguồn ESP32 (đã có)
├── node-red/        # Node-RED flow (đã có)
├── mosquitto/       # Cấu hình Mosquitto (đã có)
└── README.md        # Tài liệu tổng quan dự án
```

---

## 4. Build AOSP Android TV cho Raspberry Pi 4

> **Lưu ý quan trọng**
> Mã nguồn AOSP rất lớn (~100GB) nên không đưa lên Git.
> Repo này cung cấp **manifest lock phiên bản**, **patch**, và **script** để rebuild đúng môi trường đã dùng trong đồ án.

### 4.1. Môi trường yêu cầu

* Ubuntu 20.04 hoặc 22.04 (64-bit)
* RAM ≥ 16GB (khuyến nghị 32GB)
* Ổ cứng trống ≥ 200GB
* OpenJDK 17

Chi tiết: `docs/ENV.md`

### 4.2. Cài đặt công cụ `repo`

```bash
mkdir -p ~/.bin
curl -fsSL https://storage.googleapis.com/git-repo-downloads/repo > ~/.bin/repo
chmod +x ~/.bin/repo
echo 'export PATH="$HOME/.bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### 4.3. Tải source AOSP đúng phiên bản đã sử dụng trong đồ án

Phiên bản AOSP:

* Android 14
* Tag: `android-14.0.0_r67`
* Manifest gốc: [https://android.googlesource.com/platform/manifest](https://android.googlesource.com/platform/manifest)

Thực hiện:

```bash
cd ~/GatewayIoT
./scripts/init_repo.sh
```

Script sẽ chạy:

* `repo init` đúng tag Android 14
* `repo sync` tải toàn bộ source AOSP về `~/aosp/source`

### 4.4. Áp dụng patch cho Raspberry Pi 4

```bash
cd ~/GatewayIoT
./scripts/apply_patches.sh
```

Ghi chú:

* Patch áp dụng bằng `git am`
* Thư mục `patches/` chứa thay đổi cho Raspberry Pi 4
* File `manifest/manifest_locked_android-14.0.0_r67.xml` dùng để đối chiếu phiên bản AOSP

### 4.5. Build AOSP

```bash
cd ~/GatewayIoT
./scripts/build.sh
```

Target build:

* `aosp_rpi4_tv-ap2a-userdebug`

Output:

* `~/aosp/source/out/target/product/...`

---

## 5. Tạo image và flash vào Raspberry Pi 4

### 5.1. Thư mục output (tham khảo)

```bash
ls -lah ~/aosp/source/out/target/product/rpi4/
```

Các file thường gặp:

* `boot.img`
* `system.img`
* `vendor.img`
* `userdata.img`

### 5.2. Tạo flashable image bằng script `rpi4-mkimg.sh`

Sau khi build xong, tạo image flash SD:

```bash
cd ~/aosp/source
./rpi4-mkimg.sh
```

> `rpi4-mkimg.sh` chỉ chạy sau khi build thành công.

### 5.3. Flash image bằng Raspberry Pi Imager

1. Cài Raspberry Pi Imager (trang chính thức):
   [https://www.raspberrypi.com/software/](https://www.raspberrypi.com/software/)

2. Flash:

* **CHOOSE DEVICE** → Raspberry Pi 4
* **CHOOSE OS** → Use custom
* Chọn file image đã tạo (ví dụ: `rpi4_android_tv.img`)
* **CHOOSE STORAGE** → chọn thẻ SD
* **WRITE**

3. Boot:

* Gắn thẻ SD vào Raspberry Pi 4 → cấp nguồn → vào Android TV

---

## 6. ADB debug trên Raspberry Pi 4

### 6.1. Cài adb trên Ubuntu

```bash
sudo apt update
sudo apt install -y android-tools-adb
adb version
```

### 6.2. Bật Developer options và ADB trên Android TV (RPi4)

Trên Android TV:

* Settings → Device Preferences → About → nhấn 7 lần “Build number”
* Quay lại Developer options → bật USB debugging
* Bật ADB over network (nếu có)

### 6.3. Kết nối ADB qua LAN

```bash
adb connect <IP_RPI>:5555
adb devices
```

Kỳ vọng:

```text
List of devices attached
<IP_RPI>:5555    device
```

---

## 7. Cài Node-RED và Mosquitto trên Gateway bằng Termux

### 7.1. Cài Termux bằng APK (không lưu APK trong repo vì giới hạn 100MB của GitHub)

Tải 2 APK từ GitHub Releases của Termux:

* [https://github.com/termux/termux-app/releases](https://github.com/termux/termux-app/releases)

Cài bằng ADB:

```bash
adb connect <IP_RPI>:5555

adb push com.termux_1022.apk /data/local/tmp/
adb push com.termux.api_1002.apk /data/local/tmp/

adb install /data/local/tmp/com.termux_1022.apk
adb install /data/local/tmp/com.termux.api_1002.apk
```

### 7.2. Chuẩn bị môi trường Termux

Trong Termux:

```bash
pkg update && pkg upgrade -y
pkg install -y git nodejs python mosquitto
```

---

## 8. Mosquitto MQTT Broker

### 8.1. File cấu hình

Repo cung cấp file:

* `mosquitto/mosquitto.conf`

Trên Raspberry Pi (Termux), bạn đặt file này tại:

* `~/mosquitto.conf`

(VD: bạn có thể copy-paste nội dung từ repo vào đúng đường dẫn trên thiết bị.)

### 8.2. Chạy Mosquitto (đúng như đồ án)

```bash
mosquitto -c ~/mosquitto.conf -v
```

---

## 9. Node-RED (cài local directory)

### 9.1. Cài Node-RED local trong thư mục `~/node-red`

Trong Termux:

```bash
mkdir -p ~/node-red
cd ~/node-red
npm init -y
npm install node-red
```

### 9.2. Chạy Node-RED

```bash
cd ~/node-red
npx node-red
```

Mở Node-RED Editor:

* Trên Gateway: `http://127.0.0.1:1880/`
* Trong LAN: `http://<IP_RPI>:1880/`

---

## 10. Node-RED Flow (Import + Dashboard)

### 10.1. File flow

Repo cung cấp:

* `node-red/flows_iot_demo_ui_v3_full.json`

### 10.2. Import flow

* Mở `http://<IP_RPI>:1880/`
* Menu (góc phải) → **Import** → chọn **File** hoặc **Clipboard**
* Import file: `node-red/flows_iot_demo_ui_v3_full.json`
* **Deploy**

### 10.3. Dashboard

* `http://<IP_RPI>:1880/ui`

### 10.4. Topic/payload dùng trong demo

* Sensor: `gateway/demo/dht` → payload JSON: `{"temp":..,"hum":..}`
* LED set: `gateway/demo/led/set` → `ON` / `OFF`
* LED state: `gateway/demo/led/state` (retain) → `ON` / `OFF`
* Button event: `gateway/demo/switch` → `PRESSED`
* Status (LWT): `gateway/demo/status` → `ONLINE` / `OFFLINE`

### 10.5. Kiểm thử nhanh bằng CLI trong Termux

```bash
# subscribe toàn bộ demo topic
mosquitto_sub -v -h 127.0.0.1 -t "gateway/demo/#"

# giả lập dữ liệu DHT
mosquitto_pub -h 127.0.0.1 -t "gateway/demo/dht" -m '{"temp":30.5,"hum":70.0}'

# bật/tắt LED
mosquitto_pub -h 127.0.0.1 -t "gateway/demo/led/set" -m "ON"
mosquitto_pub -h 127.0.0.1 -t "gateway/demo/led/set" -m "OFF"
```

---

## 11. ESP32 + MQTT (Firmware)

### 11.1. Mã nguồn

Repo cung cấp:

* `esp32/esp32_gateway_demo.ino`

### 11.2. Chức năng firmware

* Kết nối Wi-Fi
* Kết nối MQTT (Broker trên Gateway)
* Publish DHT11 lên `gateway/demo/dht` mỗi ~2s
* Subscribe `gateway/demo/led/set` để điều khiển LED
* Publish LED state retain `gateway/demo/led/state`
* Publish `gateway/demo/status` (LWT ONLINE/OFFLINE)
* Publish sự kiện nút nhấn `gateway/demo/switch` (PRESSED)

### 11.3. Cần chỉnh trước khi nạp

Trong file `.ino`, chỉnh đúng:

* `WIFI_SSID`, `WIFI_PASS`
* `MQTT_HOST` (IP Gateway, ví dụ: `192.168.1.8`)
* `MQTT_PORT` (mặc định `1883`)

### 11.4. Mapping chân phần cứng (theo code)

* DHT11 (module 3 chân): DATA → GPIO 4, VCC → 3V3, GND → GND
* LED: GPIO 2 → điện trở 220–330Ω → chân dài LED; chân ngắn → GND
* Nút nhấn: 1 chân → GPIO 27; chân đối diện → GND (INPUT_PULLUP)

---

## 12. Quy trình demo tổng hợp

1. Build AOSP → tạo image → flash SD → boot Android TV trên RPi4
2. Bật ADB over network → kết nối ADB từ PC
3. Cài Termux APK
4. Trong Termux: cài Mosquitto + Node-RED
5. Chạy Mosquitto: `mosquitto -c ~/mosquitto.conf -v`
6. Chạy Node-RED → import flow → mở Dashboard `/ui`
7. Nạp firmware ESP32 (trỏ MQTT_HOST về IP Gateway)
8. Quan sát nhiệt độ/độ ẩm trên Dashboard và thử bật/tắt LED
