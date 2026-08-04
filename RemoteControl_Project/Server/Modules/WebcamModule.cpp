#include "WebcamModule.h"

#include <QDebug>
#include <QBuffer>
#include <QImage>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dshow.h>
#endif

WebcamModule::WebcamModule(QObject *parent)
    : QObject(parent), streaming(false)
{
    streamTimer = new QTimer(this);

    connect(streamTimer, &QTimer::timeout, this, [this]()
    {
        QString base64 = captureWebcam();
        if (!base64.isEmpty())
        {
            emit frameCaptured(base64);
        }
    });
}

WebcamModule::~WebcamModule()
{
    stopStream();
}


//---------------------------------------------------
// Chụp webcam 1 lần
// Sử dụng OpenCV hoặc Windows API
// Ở đây dùng cách đơn giản: capture qua command line
//---------------------------------------------------

QString WebcamModule::captureWebcam()
{
    // Sử dụng cách portable: chụp qua ffmpeg hoặc
    // trả về placeholder nếu không có webcam library

#ifdef Q_OS_WIN
    // Thử dùng ffmpeg để chụp 1 frame từ webcam
    QProcess process;
    QString tempFile = QDir::tempPath() + "/webcam_capture.jpg";

    // Dùng ffmpeg capture 1 frame
    process.start("ffmpeg",
                  QStringList() << "-y"
                                << "-f" << "dshow"
                                << "-i" << "video=0"
                                << "-frames:v" << "1"
                                << tempFile);
    process.waitForFinished(5000);

    QFile file(tempFile);
    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        file.close();
        file.remove();

        return data.toBase64();
    }
#endif

    // Fallback: tạo ảnh placeholder
    QImage placeholderImg(320, 240, QImage::Format_RGB888);
    placeholderImg.fill(Qt::darkGray);

    // Vẽ text "No Webcam"
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    placeholderImg.save(&buffer, "JPEG", 80);

    qDebug() << "Webcam capture (placeholder)";
    return byteArray.toBase64();
}


//---------------------------------------------------
// Bắt đầu stream webcam
//---------------------------------------------------

void WebcamModule::startStream()
{
    if (streaming)
    {
        qDebug() << "Webcam stream already active!";
        return;
    }

    streaming = true;
    streamTimer->start(Constants::WEBCAM_REFRESH_RATE);

    qDebug() << "Webcam stream started.";
}


//---------------------------------------------------
// Dừng stream webcam
//---------------------------------------------------

void WebcamModule::stopStream()
{
    if (!streaming)
    {
        return;
    }

    streaming = false;
    streamTimer->stop();

    qDebug() << "Webcam stream stopped.";
}


//---------------------------------------------------
// Kiểm tra trạng thái
//---------------------------------------------------

bool WebcamModule::isStreamActive() const
{
    return streaming;
}
