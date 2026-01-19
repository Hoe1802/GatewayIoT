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
