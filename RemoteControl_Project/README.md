# Remote Computer Control Project

## Mô tả
Ứng dụng điều khiển máy tính từ xa qua mạng TCP, xây dựng bằng Qt C++.

## Kiến trúc
- **Server**: Chạy trên máy bị điều khiển (Console Application)
- **Client**: Chạy trên máy điều khiển (GUI Application với TabWidget)
- **Common**: Shared code (Protocol, Packet, Constants)

## Chức năng

| Tab | Mô tả |
|-----|-------|
| Application | Liệt kê, mở, đóng ứng dụng đã cài đặt |
| Process | Liệt kê, kill, start tiến trình |
| Screenshot | Chụp màn hình, xem live screen |
| Keylogger | Ghi phím bấm từ xa |
| File Explorer | Duyệt file, download file |
| Webcam | Chụp webcam, stream webcam |
| Power | Shutdown, Restart, Sleep, Log Off |

## Giao thức truyền
- TCP Socket, port 8080
- Packet format: `[4 bytes size header][QDataStream serialized data]`
- Dữ liệu binary (ảnh) được encode base64

## Yêu cầu
- Qt 6.x
- Windows (Server sử dụng WinAPI)
- Compiler: MSVC hoặc MinGW

## Build
```bash
# Server
cd Server
qmake && make

# Client  
cd Client
qmake && make
```

## Cấu trúc dự án
```
RemoteControl_Project/
├── Common/
│   ├── Constants.h      # Cấu hình hằng số
│   ├── Packet.h         # Lớp Packet (serialize/deserialize)
│   └── Protocol.h       # Định nghĩa protocol commands
├── Server/
│   ├── main.cpp
│   ├── Network/
│   │   ├── ServerSocket.h/.cpp
│   ├── Command/
│   │   ├── CommandManager.h/.cpp
│   └── Modules/
│       ├── ApplicationModule.h/.cpp
│       ├── ProcessModule.h/.cpp
│       ├── ScreenshotModule.h/.cpp
│       ├── KeyloggerModule.h/.cpp
│       ├── FileModule.h/.cpp
│       ├── WebcamModule.h/.cpp
│       └── PowerModule.h/.cpp
├── Client/
│   ├── main.cpp
│   ├── Network/
│   │   ├── ClientSocket.h/.cpp
│   ├── GUI/
│   │   ├── MainWindow.h/.cpp/.ui
│   └── Pages/
│       ├── ApplicationPage.h/.cpp
│       ├── ProcessPage.h/.cpp
│       ├── ScreenshotPage.h/.cpp
│       ├── KeyloggerPage.h/.cpp
│       ├── FilePage.h/.cpp
│       ├── WebcamPage.h/.cpp
│       └── PowerPage.h/.cpp
```
