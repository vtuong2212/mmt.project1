#ifndef SCREENSHOTMODULE_H
#define SCREENSHOTMODULE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QByteArray>

#include "../../Common/Constants.h"

class ScreenshotModule : public QObject
{
    Q_OBJECT

public:
    explicit ScreenshotModule(QObject *parent = nullptr);
    ~ScreenshotModule();

    // Chụp màn hình 1 lần, trả về base64
    QString captureScreen();

    // Bắt đầu live screen (chụp liên tục)
    void startLiveScreen();

    // Dừng live screen
    void stopLiveScreen();

    // Kiểm tra trạng thái live
    bool isLiveActive() const;

signals:
    // Signal phát khi có frame mới (live mode)
    void frameCaptured(const QString& base64Data);

private:
    QTimer* liveTimer;
    bool liveActive;
};

#endif // SCREENSHOTMODULE_H
