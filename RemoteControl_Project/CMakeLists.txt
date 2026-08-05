cmake_minimum_required(VERSION 3.16)

project(RemoteControl VERSION 1.0 LANGUAGES CXX)

#============================================
# Cấu hình C++ tiêu chuẩn
#============================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#============================================
# Cấu hình Qt
#============================================
set(CMAKE_AUTOMOC ON)   # Tự động chạy MOC cho Q_OBJECT
set(CMAKE_AUTOUIC ON)   # Tự động xử lý file .ui
set(CMAKE_AUTORCC ON)   # Tự động xử lý file .qrc (nếu có)

# Tìm Qt6 (Core, Network, Widgets, Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Gui Multimedia)

#============================================
# Include Common headers cho cả 2 targets
#============================================
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common)

#============================================
# Sub-projects
#============================================
add_subdirectory(Server)
add_subdirectory(Client)
