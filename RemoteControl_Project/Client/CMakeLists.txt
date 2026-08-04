#============================================
# Remote Control Client (GUI Application)
#============================================

# Source files
set(CLIENT_SOURCES
    main.cpp
    Network/ClientSocket.cpp
    GUI/MainWindow.cpp
    Pages/ApplicationPage.cpp
    Pages/ProcessPage.cpp
    Pages/ScreenshotPage.cpp
    Pages/KeyloggerPage.cpp
    Pages/FilePage.cpp
    Pages/WebcamPage.cpp
    Pages/PowerPage.cpp
)

# Header files (cần cho MOC xử lý Q_OBJECT)
set(CLIENT_HEADERS
    Network/ClientSocket.h
    GUI/MainWindow.h
    Pages/ApplicationPage.h
    Pages/ProcessPage.h
    Pages/ScreenshotPage.h
    Pages/KeyloggerPage.h
    Pages/FilePage.h
    Pages/WebcamPage.h
    Pages/PowerPage.h
)

# UI files
set(CLIENT_UI
    GUI/MainWindow.ui
)

# Tạo executable
add_executable(RemoteControlClient
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
    ${CLIENT_UI}
)

# Include thư mục Common
target_include_directories(RemoteControlClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common
)

# Link Qt libraries (Widgets bao gồm cả Core)
target_link_libraries(RemoteControlClient PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
)
