// protocal.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>

namespace Protocol
{
    const QString LIST_APPLICATION = "LIST_APPLICATION";
    const QString OPEN_APPLICATION = "OPEN_APPLICATION";
    const QString CLOSE_APPLICATION = "CLOSE_APPLICATION";

    const QString LIST_PROCESS = "LIST_PROCESS";
    const QString KILL_PROCESS = "KILL_PROCESS";
    const QString START_PROCESS = "START_PROCESS";
    const QString SCREENSHOT = "SCREENSHOT";
    const QString START_LIVE_SCREEN = "START_LIVE_SCREEN";
    const QString STOP_LIVE_SCREEN = "STOP_LIVE_SCREEN";

    const QString START_KEYLOGGER = "START_KEYLOGGER";
    const QString STOP_KEYLOGGER = "STOP_KEYLOGGER";
    const QString GET_KEYLOGGER_DATA = "GET_KEYLOGGER_DATA";


    const QString LIST_FILES = "LIST_FILES";
    const QString DOWNLOAD_FILE = "DOWNLOAD_FILE";

    // web cam
    const QString CAPTURE_WEBCAM = "CAPTURE_WEBCAM";
    const QString START_WEBCAM_STREAM = "START_WEBCAM_STREAM";
    const QString STOP_WEBCAM_STREAM = "STOP_WEBCAM_STREAM";

    // power control
    const QString SHUTDOWN = "SHUTDOWN";
    const QString RESTART = "RESTART";
    const QString SLEEP = "SLEEP";
    const QString LOG_OFF = "LOG_OFF";
    // connection command 
    const QString CONNECT = "CONNECT";
    const QString DISCONNECT = "DISCONNECT";

    // response from other sever
    const QString SUCCESS = "SUCCESS";
    const QString FAIL = "FAIL";
    const QString ERROR = "ERROR";
}

#endif // PROTOCOL_H
