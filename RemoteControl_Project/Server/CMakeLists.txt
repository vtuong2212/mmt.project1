#============================================
# Remote Control Server (Console Application)
#============================================

# Source files
set(SERVER_SOURCES
    main.cpp
    Network/ServerSocket.cpp
    Command/CommandManager.cpp
    Modules/ApplicationModule.cpp
    Modules/ProcessModule.cpp
    Modules/ScreenshotModule.cpp
    Modules/KeyloggerModule.cpp
    Modules/FileModule.cpp
    Modules/WebcamModule.cpp
    Modules/PowerModule.cpp
)

# Header files (cần cho MOC xử lý Q_OBJECT)
set(SERVER_HEADERS
    Network/ServerSocket.h
    Command/CommandManager.h
    Modules/ApplicationModule.h
    Modules/ProcessModule.h
    Modules/ScreenshotModule.h
    Modules/KeyloggerModule.h
    Modules/FileModule.h
    Modules/WebcamModule.h
    Modules/PowerModule.h
)

# Tạo executable
add_executable(RemoteControlServer
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
)

# Include thư mục Common
target_include_directories(RemoteControlServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common
)

# Link Qt libraries (Gui cho Screenshot, Multimedia cho Webcam)
target_link_libraries(RemoteControlServer PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

#============================================
# Link thêm thư viện hệ thống cho Windows
# (WinAPI cho Keylogger, Screenshot, v.v.)
#============================================
if(WIN32)
    target_link_libraries(RemoteControlServer PRIVATE
        user32
        gdi32
        advapi32
        powrprof
        strmiids
        ole32
    )
endif()
