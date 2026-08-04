#ifndef WEBCAMMODULE_H
#define WEBCAMMODULE_H

#include <QObject>
#include <QString>
#include <QTimer>

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
    QTimer* streamTimer;
    bool streaming;
};

#endif // WEBCAMMODULE_H
