#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QString>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

#include "../Modules/ApplicationModule.h"
#include "../Modules/ProcessModule.h"
#include "../Modules/ScreenshotModule.h"
#include "../Modules/KeyloggerModule.h"
#include "../Modules/FileModule.h"
#include "../Modules/WebcamModule.h"
#include "../Modules/PowerModule.h"

class CommandManager : public QObject
{
    Q_OBJECT

public:
    explicit CommandManager(QObject *parent = nullptr);
    ~CommandManager();

public slots:
    // Xử lý command nhận được từ Client
    void handleCommand(const Packet& packet);

signals:
    // Signal phát khi có response cần gửi về Client
    void responseReady(const Packet& packet);

private:
    ApplicationModule* appModule;
    ProcessModule* processModule;
    ScreenshotModule* screenshotModule;
    KeyloggerModule* keyloggerModule;
    FileModule* fileModule;
    WebcamModule* webcamModule;
    PowerModule* powerModule;
};

#endif // COMMANDMANAGER_H
