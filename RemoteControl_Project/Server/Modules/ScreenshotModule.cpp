#include "ScreenshotModule.h"

#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QDebug>

ScreenshotModule::ScreenshotModule(QObject *parent)
    : QObject(parent), liveActive(false)
{
    liveTimer = new QTimer(this);

    connect(liveTimer, &QTimer::timeout, this, [this]()
    {
        QString base64 = captureScreen();
        if (!base64.isEmpty())
        {
            emit frameCaptured(base64);
        }
    });
}

ScreenshotModule::~ScreenshotModule()
{
    stopLiveScreen();
}


//---------------------------------------------------
// Chụp màn hình, trả về ảnh base64 (JPEG)
//---------------------------------------------------

QString ScreenshotModule::captureScreen()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen)
    {
        qDebug() << "No screen found!";
        return "";
    }

    QPixmap pixmap = screen->grabWindow(0);

    if (pixmap.isNull())
    {
        qDebug() << "Failed to capture screen!";
        return "";
    }

    // Encode thành JPEG base64 (nhỏ hơn PNG nhiều)
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPEG", 50);  // Quality 50 để giảm dung lượng

    QString base64 = byteArray.toBase64();

    qDebug() << "Screenshot captured, size:" << base64.size() << "bytes";
    return base64;
}


//---------------------------------------------------
// Bắt đầu live screen
//---------------------------------------------------

void ScreenshotModule::startLiveScreen()
{
    if (liveActive)
    {
        qDebug() << "Live screen already active!";
        return;
    }

    liveActive = true;
    liveTimer->start(Constants::SCREEN_REFRESH_RATE);

    qDebug() << "Live screen started, interval:"
             << Constants::SCREEN_REFRESH_RATE << "ms";
}


//---------------------------------------------------
// Dừng live screen
//---------------------------------------------------

void ScreenshotModule::stopLiveScreen()
{
    if (!liveActive)
    {
        return;
    }

    liveActive = false;
    liveTimer->stop();

    qDebug() << "Live screen stopped.";
}


//---------------------------------------------------
// Kiểm tra trạng thái
//---------------------------------------------------

bool ScreenshotModule::isLiveActive() const
{
    return liveActive;
}
