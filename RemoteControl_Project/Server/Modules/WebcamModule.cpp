#include "WebcamModule.h"

#include <QDebug>
#include <QBuffer>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCoreApplication>
#include <QElapsedTimer>

WebcamModule::WebcamModule(QObject *parent)
    : QObject(parent),
      camera(nullptr),
      captureSession(nullptr),
      videoSink(nullptr),
      streaming(false),
      cameraInitialized(false)
{
    streamTimer = new QTimer(this);

    // Khi timer tick, gửi frame mới nhất cho client
    connect(streamTimer, &QTimer::timeout, this, [this]()
    {
        if (!latestFrame.isNull())
        {
            emit frameCaptured(imageToBase64(latestFrame));
        }
    });
}

WebcamModule::~WebcamModule()
{
    stopStream();

    if (camera)
    {
        camera->stop();
    }
}


//---------------------------------------------------
// Khởi tạo camera (gọi 1 lần, lazy init)
// Dùng QCamera + QVideoSink từ Qt Multimedia
//---------------------------------------------------

void WebcamModule::initCamera()
{
    if (cameraInitialized)
    {
        return;
    }

    // Kiểm tra có camera nào không
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty())
    {
        qDebug() << "No webcam device found!";
        return;
    }

    qDebug() << "Found" << cameras.size() << "camera(s):";
    for (const QCameraDevice& cam : cameras)
    {
        qDebug() << "  -" << cam.description();
    }

    // Tạo camera với device đầu tiên
    camera = new QCamera(cameras.first(), this);
    captureSession = new QMediaCaptureSession(this);
    videoSink = new QVideoSink(this);

    // Kết nối pipeline: Camera → CaptureSession → VideoSink
    captureSession->setCamera(camera);
    captureSession->setVideoSink(videoSink);

    // Nhận frame từ VideoSink
    connect(videoSink, &QVideoSink::videoFrameChanged,
            this, [this](const QVideoFrame& frame)
    {
        QVideoFrame f = frame;  // Cần bản copy non-const

        if (f.isValid())
        {
            latestFrame = f.toImage();
        }
    });

    // Bắt đầu camera
    camera->start();
    cameraInitialized = true;

    qDebug() << "Camera initialized:"
             << cameras.first().description();
}


//---------------------------------------------------
// Chụp webcam 1 lần, trả về base64 JPEG
//---------------------------------------------------

QString WebcamModule::captureWebcam()
{
    initCamera();

    // Nếu camera chưa có frame, đợi tối đa 3 giây
    if (latestFrame.isNull())
    {
        QElapsedTimer timer;
        timer.start();

        while (latestFrame.isNull() && timer.elapsed() < 3000)
        {
            QCoreApplication::processEvents(
                QEventLoop::AllEvents, 100);
        }
    }

    if (!latestFrame.isNull())
    {
        qDebug() << "Webcam captured, size:"
                 << latestFrame.size();
        return imageToBase64(latestFrame);
    }

    // Fallback: ảnh placeholder nếu không có camera
    qDebug() << "Webcam capture failed - returning placeholder";

    QImage placeholder(320, 240, QImage::Format_RGB888);
    placeholder.fill(Qt::darkGray);
    return imageToBase64(placeholder);
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

    initCamera();

    streaming = true;
    streamTimer->start(Constants::WEBCAM_REFRESH_RATE);

    qDebug() << "Webcam stream started, interval:"
             << Constants::WEBCAM_REFRESH_RATE << "ms";
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


//---------------------------------------------------
// Chuyển QImage → base64 JPEG string
//---------------------------------------------------

QString WebcamModule::imageToBase64(const QImage& image)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    // JPEG quality 50 để giảm dung lượng truyền qua mạng
    image.save(&buffer, "JPEG", 50);

    return byteArray.toBase64();
}
