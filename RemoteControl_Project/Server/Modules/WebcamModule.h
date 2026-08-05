#ifndef WEBCAMMODULE_H
#define WEBCAMMODULE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QImage>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>

#include "../../Common/Constants.h"

class WebcamModule : public QObject
{
    Q_OBJECT

public:
    explicit WebcamModule(QObject *parent = nullptr);
    ~WebcamModule();

    // Chụp webcam 1 lần, trả về base64
    QString captureWebcam();

    // Bắt đầu stream webcam
    void startStream();

    // Dừng stream webcam
    void stopStream();

    // Kiểm tra trạng thái
    bool isStreamActive() const;

signals:
    // Signal phát khi có frame mới (stream mode)
    void frameCaptured(const QString& base64Data);

private:
    // Khởi tạo camera (lazy init)
    void initCamera();

    // Chuyển QImage → base64 JPEG
    QString imageToBase64(const QImage& image);

    QCamera* camera;
    QMediaCaptureSession* captureSession;
    QVideoSink* videoSink;
    QTimer* streamTimer;

    QImage latestFrame;     // Frame mới nhất từ camera
    bool streaming;
    bool cameraInitialized;
};

#endif // WEBCAMMODULE_H
