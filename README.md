# Gateway IoT trên nền tảng AOSP Android TV cho Raspberry Pi 4

## 1. Giới thiệu
Đồ án xây dựng một **Gateway IoT** chạy trên **Raspberry Pi 4**, sử dụng **AOSP Android TV (Android 14)** làm hệ điều hành nền tảng.  
Gateway đóng vai trò trung gian giữa **thiết bị IoT (ESP32 + cảm biến)** và **người dùng**, thực hiện các chức năng:

- Thu thập dữ liệu cảm biến từ thiết bị IoT thông qua giao thức MQTT
- Xử lý dữ liệu tại biên (Edge Computing) ngay trên Gateway
- Hiển thị dữ liệu thời gian thực qua Dashboard
- Điều khiển thiết bị IoT từ xa
- Tích hợp **Node-RED** để xử lý luồng dữ liệu
- Sử dụng **Mosquitto MQTT Broker** cho cơ chế publish/subscribe

Toàn bộ hệ thống được triển khai và kiểm thử trên **Raspberry Pi 4**.

---

## 2. Kiến trúc tổng thể hệ thống
Hệ thống Gateway IoT bao gồm các thành phần chính:

- **ESP32**: Thu thập dữ liệu cảm biến (nhiệt độ, độ ẩm, trạng thái nút nhấn) và gửi dữ liệu qua MQTT
- **Gateway (Raspberry Pi 4)**:
  - Chạy **AOSP Android TV**
  - Chạy **Mosquitto MQTT Broker**
  - Chạy **Node-RED** để xử lý dữ liệu và điều khiển
- **Người dùng**:
  - Theo dõi dữ liệu
  - Gửi lệnh điều khiển thông qua Dashboard

---

## 3. Cấu trúc thư mục dự án
GatewayIoT/
├── manifest/ # Manifest AOSP đã lock phiên bản
├── patches/ # Patch thay đổi AOSP cho Raspberry Pi 4
├── scripts/ # Script init source, apply patch, build AOSP
├── docs/ # Tài liệu môi trường và hướng dẫn chi tiết
├── esp32/ # Mã nguồn ESP32 (sẽ bổ sung)
├── node-red/ # Node-RED flows (sẽ bổ sung)
├── mosquitto/ # Cấu hình Mosquitto (sẽ bổ sung)
└── README.md # Tài liệu tổng quan dự án

---

## 4. Build AOSP Android TV cho Raspberry Pi 4

> **Lưu ý quan trọng**  
> Mã nguồn AOSP có dung lượng rất lớn (~100GB) nên **không được nộp trực tiếp**.  
> Repo này cung cấp **manifest đã khóa phiên bản**, **patch**, và **script** để rebuild lại đúng phiên bản AOSP đã sử dụng trong đồ án.

---

### 4.1. Môi trường yêu cầu
- Ubuntu 20.04 hoặc 22.04 (64-bit)
- RAM ≥ 16GB (khuyến nghị 32GB)
- Ổ cứng trống ≥ 200GB
- OpenJDK 17

Chi tiết xem: `docs/ENV.md`

---

### 4.2. Cài đặt công cụ `repo`
```bash
mkdir -p ~/.bin
curl -fsSL https://storage.googleapis.com/git-repo-downloads/repo > ~/.bin/repo
chmod +x ~/.bin/repo
echo 'export PATH="$HOME/.bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
###4.3. Tải source AOSP đúng phiên bản đã sử dụng trong đồ án

Phiên bản AOSP:

Android 14 – tag android-14.0.0_r67

Manifest gốc: https://android.googlesource.com/platform/manifest
cd GatewayIoT
./scripts/init_repo.sh
Script thực hiện:

repo init với đúng manifest và tag

repo sync để tải toàn bộ source AOSP
4.4. Áp dụng patch cho Raspberry Pi 4
cd GatewayIoT
./scripts/apply_patches.sh
Ghi chú:

Các patch được áp dụng bằng git am

Thư mục patches/ chứa toàn bộ thay đổi cho Raspberry Pi 4

File manifest/manifest_locked_android-14.0.0_r67.xml dùng để đối chiếu phiên bản AOSP đã build

###4.5. Build AOSP


cd GatewayIoT
./scripts/build.sh


Target build:

aosp_rpi4_tv-ap2a-userdebug


Kết quả build nằm trong:

~/aosp/source/out/target/product/...

---

### 4.6. Tạo image để flash cho Raspberry Pi 4

Sau khi build thành công, AOSP sẽ tạo ra các image cần thiết cho Raspberry Pi 4
trong thư mục output.

#### 4.6.1. Thư mục output
```bash
~/aosp/source/out/target/product/rpi4/

Các file quan trọng:

boot.img – kernel + ramdisk

system.img – hệ thống Android

vendor.img – vendor partition

userdata.img – dữ liệu người dùng

*.img – image dùng để flash cho Raspberry Pi 4 (tùy cấu hình build)

Tên file image có thể thay đổi tùy cấu hình device, nhưng đều nằm trong thư mục trên.


#### 4.6.2 Tạo flashimage
Đối với AOSP trên Raspberry Pi 4, image dùng để flash vào thẻ SD
được tạo thông qua script chuyên dụng `rpi4-mkimg.sh`.

Script này sẽ gom các partition cần thiết (boot, system, vendor, userdata…)
thành một file image duy nhất để flash cho Raspberry Pi 4.

Thực hiện:

```bash
cd ~/aosp/source
./rpi4-mkimg.sh



---

## 📌 LƯU Ý KỸ THUẬT (rất nên giữ trong README)
- `rpi4-mkimg.sh` **chỉ chạy sau khi build AOSP thành công**
- Đây là bước **bắt buộc** để tạo image flashable cho thẻ SD


###4.7. Flash AOSP image vào Raspberry Pi 4 bằng Raspberry Pi Imager

Trong đồ án, image AOSP được flash vào thẻ SD
bằng công cụ Raspberry Pi Imager (khuyến nghị sử dụng).

####4.7.1. Chuẩn bị

Raspberry Pi 4

Thẻ SD ≥ 16GB

Đầu đọc thẻ SD

Máy tính cài Raspberry Pi Imager

####4.7.2. Cài đặt Raspberry Pi Imager

Tải Raspberry Pi Imager từ trang chính thức:

https://www.raspberrypi.com/software/


Cài đặt theo hướng dẫn cho hệ điều hành tương ứng
(Ubuntu / Windows / macOS).

####4.7.3. Flash image bằng Raspberry Pi Imager

Thực hiện các bước sau:

Mở Raspberry Pi Imager

Chọn CHOOSE DEVICE → Raspberry Pi 4

Chọn CHOOSE OS → Use custom

Chọn file image đã tạo:

rpi4_android_tv.img


Chọn CHOOSE STORAGE → thẻ SD

Nhấn WRITE để bắt đầu flash

Quá trình flash sẽ mất vài phút tùy tốc độ thẻ SD.

###4.7.4. Khởi động Raspberry Pi 4

Tháo thẻ SD khỏi máy tính

Gắn thẻ SD vào Raspberry Pi 4

Kết nối màn hình HDMI

Cấp nguồn cho Raspberry Pi 4

Nếu build và flash thành công:

Thiết bị sẽ khởi động vào Android TV

Giao diện hiển thị trên màn hình

Có thể tiếp tục cấu hình mạng và các dịch vụ IoT
---

### 4.8. ADB debug và kiểm thử trên Raspberry Pi 4

ADB (Android Debug Bridge) được sử dụng để kiểm tra, debug và tương tác
với hệ thống AOSP Android TV đang chạy trên Raspberry Pi 4.

---

#### 4.8.1. Chuẩn bị
- Raspberry Pi 4 đã boot vào Android TV
- Raspberry Pi 4 và máy tính cùng mạng LAN
- Máy tính đã cài Android Platform Tools (adb)

Cài adb trên Ubuntu:
```bash
sudo apt update
sudo apt install -y android-tools-adb
adb version
####4.8.2. Bật ADB trên Android TV (RPi4)

Trên giao diện Android TV:

Vào Settings

Chọn Device Preferences

Chọn About

Nhấn liên tục 7 lần vào Build number để bật Developer options

Quay lại Device Preferences

Vào Developer options

Bật USB debugging

Bật ADB over network (nếu có)

####4.8.3. Kết nối ADB qua mạng (ADB over TCP/IP)

Trên Raspberry Pi 4, xác định địa chỉ IP:

Vào Settings → Network

Hoặc kiểm tra qua router

Giả sử IP của Raspberry Pi 4 là 192.168.1.100.

Trên máy tính:

adb connect 192.168.1.100:5555


Kiểm tra kết nối:

adb devices


Kết quả mong đợi:

List of devices attached
192.168.1.100:5555   device


---

## 5. Cài đặt Node-RED và Mosquitto trên Gateway bằng Termux

Trong đồ án, **Node-RED** và **Mosquitto MQTT Broker** được cài đặt
và chạy trực tiếp trên **Gateway (Raspberry Pi 4 – Android TV)**
thông qua môi trường **Termux**.

Giải pháp này cho phép:
- Chạy dịch vụ IoT trực tiếp trên AOSP
- Không cần root Android
- Phù hợp cho mục đích học tập và nghiên cứu

---

### 5.1. Cài đặt Termux trên Android TV (Raspberry Pi 4)

#### 5.1.1. Chuẩn bị file APK
Trong quá trình thực hiện đồ án, Termux được cài đặt bằng **hai file APK chính thức**:

- `com.termux_1022.apk`
- `com.termux.api_1002.apk`

Các file này được tải từ:

https://github.com/termux/termux-app/releases


---

#### 5.1.2. Cài đặt Termux bằng ADB

Kết nối ADB tới Raspberry Pi 4:
```bash
adb connect <IP_RPI>:5555
adb devices

Chép APK vào thiết bị:

adb push com.termux_1022.apk /data/local/tmp/
adb push com.termux.api_1002.apk /data/local/tmp/


Cài đặt APK:

adb install /data/local/tmp/com.termux_1022.apk
adb install /data/local/tmp/com.termux.api_1002.apk


Sau khi cài đặt hoàn tất, Termux sẽ xuất hiện trong danh sách ứng dụng.


####5.1.3. Khởi động Termux

Mở ứng dụng Termux trên Android TV

Một môi trường dòng lệnh Linux user-space sẽ được cung cấp

###5.2. Chuẩn bị môi trường trong Termux

Cập nhật hệ thống gói:

pkg update
pkg upgrade


Cài các gói cần thiết:

pkg install -y nodejs git python


Kiểm tra:

node -v
npm -v


###5.3. Cài đặt Mosquitto MQTT Broker trong Termux
####5.3.1. Cài Mosquitto
pkg install -y mosquitto


Kiểm tra:

mosquitto -v

####5.3.2. Chạy Mosquitto
mosquitto


Mặc định:

Port: 1883

Broker chạy trong môi trường Termux

###5.4. Cài đặt Node-RED trong Termux (local directory)
####5.4.1. Tạo thư mục Node-RED
mkdir -p ~/node-red
cd ~/node-red


Khởi tạo project Node.js:

npm init -y

####5.4.2. Cài Node-RED (local)
npm install node-red


Kiểm tra:

npx node-red --version

####5.4.3. Chạy Node-RED
cd ~/node-red
npx node-red


Khi khởi động thành công, Node-RED sẽ hiển thị:

Server now running at http://127.0.0.1:1880/

####5.4.4. Truy cập Node-RED

Trên Raspberry Pi 4:

http://127.0.0.1:1880


Trên máy khác trong mạng LAN:

http://<IP_RPI>:1880
---

## 6. Node-RED Flow (Import, cấu hình MQTT và chạy Dashboard)

Flow Node-RED của đồ án được cung cấp dưới dạng file JSON trong thư mục:
node-red/

Flow thực hiện:
- Nhận dữ liệu DHT11 từ ESP32 qua MQTT topic `gateway/demo/dht`
- Tách `temp` và `hum` để hiển thị realtime (card) và biểu đồ 10 phút gần nhất
- Điều khiển LED qua MQTT topic `gateway/demo/led/set` với payload `ON/OFF`
- Hiển thị trạng thái LED đồng bộ từ ESP32 qua topic `gateway/demo/led/state` (retain)

### 6.1. Chạy Mosquitto và Node-RED
**Mosquitto (Termux):**
```bash
mosquitto -c ~/mosquitto.conf -v

**Node-RED (Termux):**
```bash
cd ~/node-red
npx node-red

### 6.2. Import flow

Mở Node-RED Editor:
http://<IP_GATEWAY>:1880

Menu (góc phải) → Import → Clipboard hoặc File

Chọn file flow trong repo (ví dụ đặt tên):
node-red/flows.json

Nhấn Import → Deploy

### 6.3. Cấu hình MQTT Broker trong flow

Trong flow của bạn, MQTT Broker cấu hình là:

Host: <IP>

Port: 1883

Name: Mosquitto (Gateway)

Nếu IP Gateway thay đổi, chỉ cần sửa host trong cấu hình broker của Node-RED.

### 6.4. Dashboard

Sau khi Deploy, Dashboard truy cập tại:

http://<IP_GATEWAY>:1880/ui

### 6.5. Topic/payload dùng trong flow

Sensor (ESP32 → Node-RED):

Topic: gateway/demo/dht

Payload: JSON (string hoặc object), ví dụ: {"temp":30.5,"hum":70.0}

Điều khiển LED (Node-RED → ESP32):

Topic: gateway/demo/led/set

Payload: ON hoặc OFF

Trạng thái LED (ESP32 → Node-RED, retain):

Topic: gateway/demo/led/state

Payload: ON hoặc OFF

### 6.6. Kiểm thử nhanh bằng mosquitto client

Subscribe toàn bộ topic demo:

mosquitto_sub -v -h 127.0.0.1 -t "gateway/demo/#"


Giả lập gửi dữ liệu DHT:

mosquitto_pub -h 127.0.0.1 -t "gateway/demo/dht" -m '{"temp":30.5,"hum":70.0}'


Bật/tắt LED:

mosquitto_pub -h 127.0.0.1 -t "gateway/demo/led/set" -m "ON"
mosquitto_pub -h 127.0.0.1 -t "gateway/demo/led/set" -m "OFF"

## 7. ESP32 + MQTT (Nạp code và chạy demo)

Mã nguồn ESP32 được cung cấp trong thư mục:

esp32/


Chức năng firmware:

Kết nối Wi-Fi

Kết nối MQTT broker 192.168.1.8:1883

Đọc DHT11 (GPIO 4) và publish JSON lên gateway/demo/dht mỗi ~2 giây

Nhận lệnh điều khiển LED qua gateway/demo/led/set (ON/OFF)

Publish trạng thái LED lên gateway/demo/led/state (retain)

Publish trạng thái ONLINE/OFFLINE qua LWT topic gateway/demo/status

Publish sự kiện nút nhấn lên gateway/demo/switch (payload PRESSED)

Nút nhấn dùng INPUT_PULLUP, nhấn kéo xuống GND, interrupt FALLING, debounce 200ms

### 7.1. Cấu hình cần chỉnh trong code trước khi nạp

Trong file code ESP32, chỉnh đúng:

Wi-Fi:

WIFI_SSID

WIFI_PASS

MQTT:

MQTT_HOST (IP Gateway, ví dụ 192.168.1.8)

MQTT_PORT (1883)

Topic trong code (đã đồng bộ với Node-RED flow):

gateway/demo/dht

gateway/demo/led/set

gateway/demo/led/state

gateway/demo/switch

gateway/demo/status

7.2. Mapping chân kết nối phần cứng (đúng theo code)

DHT11 (module 3 chân):

DATA → GPIO 4

VCC → 3V3

GND → GND

LED:

GPIO 2 → điện trở 220–330Ω → chân dài LED

chân ngắn LED → GND

Nút nhấn:

1 chân → GPIO 27

chân đối diện → GND

cấu hình INPUT_PULLUP (không cần điện trở ngoài)

### 7.3. Nạp firmware

Mở project trong esp32/ bằng Arduino IDE hoặc PlatformIO:

Chọn đúng board ESP32

Cắm ESP32 và Upload

### 7.4. Kiểm thử MQTT (debug)

Trên Gateway/Termux, subscribe tất cả demo topic:

mosquitto_sub -v -h 127.0.0.1 -t "gateway/demo/#"


Kiểm tra các bản tin mong đợi:

gateway/demo/status: ONLINE (retain) khi ESP32 kết nối MQTT

gateway/demo/dht: JSON {"temp":..,"hum":..} định kỳ

gateway/demo/led/state: ON/OFF (retain) khi có thay đổi

gateway/demo/switch: PRESSED khi nhấn nút

Test điều khiển LED thủ công:

mosquitto_pub -h 127.0.0.1 -t "gateway/demo/led/set" -m "ON"
mosquitto_pub -h 127.0.0.1 -t "gateway/demo/led/set" -m "OFF"

### 7.5. Quy trình demo tổng hợp

Flash AOSP lên RPi4 và boot Android TV

Cài Termux + cài Mosquitto + Node-RED

Chạy Mosquitto (Termux)

Chạy Node-RED (Termux) và import flow

Nạp code ESP32 (trỏ MQTT_HOST về IP Gateway)

Quan sát nhiệt độ/độ ẩm trên Dashboard và thử bật/tắt LED
