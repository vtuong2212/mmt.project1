// constant.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
    //=========================================
    // Network Settings
    //=========================================

    constexpr int SERVER_PORT = 8080;
    constexpr int BUFFER_SIZE = 4096;

    //=========================================
    // Screen Settings
    //=========================================

    constexpr int SCREEN_REFRESH_RATE = 500;
    // Đơn vị: milliseconds (0.5 giây)

    //=========================================
    // Webcam Settings
    //=========================================

    constexpr int WEBCAM_REFRESH_RATE = 500;
    // Đơn vị: milliseconds (0.5 giây)

    //=========================================
    // File Transfer Settings
    //=========================================

    constexpr int FILE_CHUNK_SIZE = 4096;

    //=========================================
    // Connection Settings
    //=========================================

    constexpr int CONNECTION_TIMEOUT = 5000;
    // Đơn vị: milliseconds (5 giây)

    //=========================================
    // Default Values
    //=========================================

    constexpr int MAX_CLIENTS = 1;
}

#endif // CONSTANTS_H
