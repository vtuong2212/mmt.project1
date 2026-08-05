#include "WebcamModule.h"

#include <QDebug>
#include <QBuffer>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QPermission>

WebcamModule::WebcamModule(QObject *parent)
    : QObject(parent),
      camera(nullptr),
      captureSession(nullptr),
      videoSink(nullptr),
      streaming(false),
      cameraInitialized(false),
      permissionGranted(false),
      pendingStream(false)
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
// Xin quyền Camera trên macOS
// Qt 6.5+ cung cấp QPermission API
// macOS yêu cầu NSCameraUsageDescription trong Info.plist
// VÀ phải gọi API xin quyền runtime
//---------------------------------------------------

void WebcamModule::requestCameraPermission(std::function<void(bool)> callback)
{
    // Nếu đã được cấp quyền rồi, gọi callback ngay
    if (permissionGranted)
    {
        callback(true);
        return;
    }

#if QT_CONFIG(permissions)
    QCameraPermission cameraPermission;

    // Kiểm tra trạng thái quyền hiện tại
    switch (qApp->checkPermission(cameraPermission))
    {
    case Qt::PermissionStatus::Granted:
        qDebug() << "Camera permission: GRANTED";
        permissionGranted = true;
        callback(true);
        break;

    case Qt::PermissionStatus::Undetermined:
        qDebug() << "Camera permission: UNDETERMINED - requesting...";

        // Xin quyền từ user (hiện dialog hệ thống)
        qApp->requestPermission(cameraPermission,
            [this, callback](const QPermission& permission)
        {
            if (permission.status() == Qt::PermissionStatus::Granted)
            {
                qDebug() << "Camera permission: GRANTED by user";
                permissionGranted = true;
                callback(true);
            }
            else
            {
                qDebug() << "Camera permission: DENIED by user";
                callback(false);
            }
        });
        break;

    case Qt::PermissionStatus::Denied:
        qDebug() << "Camera permission: DENIED";
        qDebug() << "Please go to System Settings > Privacy & Security > Camera"
                 << "and enable access for RemoteControlServer";
        callback(false);
        break;
    }
#else
    // Nếu Qt không hỗ trợ permission API (Qt < 6.5),
    // giả sử có quyền (Linux/Windows không cần)
    qDebug() << "Qt permission API not available, assuming granted";
    permissionGranted = true;
    callback(true);
#endif
}


//---------------------------------------------------
// Khởi tạo camera (gọi sau khi có quyền)
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

    // Log lỗi camera
    connect(camera, &QCamera::errorOccurred,
            this, [](QCamera::Error error, const QString& errorString)
    {
        qDebug() << "Camera error:" << error << errorString;
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
    // Nếu chưa có quyền, xin quyền đồng bộ
    if (!permissionGranted)
    {
        bool granted = false;

        requestCameraPermission([&granted](bool result)
        {
            granted = result;
        });

        // Đợi permission dialog (processEvents cho đến khi có kết quả)
        QElapsedTimer permTimer;
        permTimer.start();
        while (!permissionGranted && !granted && permTimer.elapsed() < 5000)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        if (!permissionGranted)
        {
            qDebug() << "Camera permission not granted - returning placeholder";
            QImage placeholder(320, 240, QImage::Format_RGB888);
            placeholder.fill(Qt::darkGray);
            return imageToBase64(placeholder);
        }
    }

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

    // Xin quyền trước khi start stream
    requestCameraPermission([this](bool granted)
    {
        if (granted)
        {
            initCamera();

            streaming = true;
            streamTimer->start(Constants::WEBCAM_REFRESH_RATE);

            qDebug() << "Webcam stream started, interval:"
                     << Constants::WEBCAM_REFRESH_RATE << "ms";
        }
        else
        {
            qDebug() << "Cannot start webcam stream - permission denied";
        }
    });
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
