#include "WebcamModule.h"

#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <QProcess>
#include <QDir>
#include <QFile>

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
    // Bước 1: Tìm tên webcam device qua ffmpeg
    QProcess listProcess;
    listProcess.start("ffmpeg",
                      QStringList() << "-list_devices" << "true"
                                    << "-f" << "dshow"
                                    << "-i" << "dummy");
    listProcess.waitForFinished(5000);

    // ffmpeg output device list vào stderr
    QString deviceOutput = listProcess.readAllStandardError();
    QString deviceName;

    // Tìm dòng chứa "video" device
    QStringList lines = deviceOutput.split("\n");
    for (const QString& line : lines)
    {
        // Format: [dshow @ ...] "Device Name" (video)
        if (line.contains("(video)"))
        {
            int firstQuote = line.indexOf('"');
            int lastQuote = line.indexOf('"', firstQuote + 1);
            if (firstQuote >= 0 && lastQuote > firstQuote)
            {
                deviceName = line.mid(firstQuote + 1,
                                      lastQuote - firstQuote - 1);
                break;
            }
        }
    }

    if (!deviceName.isEmpty())
    {
        QProcess process;
        QString tempFile = QDir::tempPath() + "/webcam_capture.jpg";

        // Dùng ffmpeg capture 1 frame với tên device chính xác
        process.start("ffmpeg",
                      QStringList() << "-y"
                                    << "-f" << "dshow"
                                    << "-i" << ("video=" + deviceName)
                                    << "-frames:v" << "1"
                                    << tempFile);
        process.waitForFinished(10000);

        QFile file(tempFile);
        if (file.exists() && file.open(QIODevice::ReadOnly))
        {
            QByteArray data = file.readAll();
            file.close();
            file.remove();

            qDebug() << "Webcam captured via ffmpeg, device:" << deviceName;
            return data.toBase64();
        }
    }
    else
    {
        qDebug() << "No webcam device found!";
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
