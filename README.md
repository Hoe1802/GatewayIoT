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
