#include "ScreenshotPage.h"
#include "../Network/ClientSocket.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QByteArray>

ScreenshotPage::ScreenshotPage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void ScreenshotPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Screenshot & Live Screen");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    btnCapture = new QPushButton("Capture Screenshot");
    btnStartLive = new QPushButton("Start Live Screen");
    btnStopLive = new QPushButton("Stop Live Screen");
    btnSave = new QPushButton("Save Image");

    btnCapture->setMinimumHeight(35);
    btnStartLive->setMinimumHeight(35);
    btnStopLive->setMinimumHeight(35);
    btnSave->setMinimumHeight(35);

    btnStartLive->setStyleSheet("background-color: #27ae60; color: white;");
    btnStopLive->setStyleSheet("background-color: #e74c3c; color: white;");

    buttonLayout->addWidget(btnCapture);
    buttonLayout->addWidget(btnStartLive);
    buttonLayout->addWidget(btnStopLive);
    buttonLayout->addWidget(btnSave);

    mainLayout->addLayout(buttonLayout);

    // Image display area
    scrollArea = new QScrollArea();
    imageLabel = new QLabel("No screenshot yet");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(640, 480);
    imageLabel->setStyleSheet("background-color: #2c3e50; color: white; "
                              "font-size: 14px;");

    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);

    mainLayout->addWidget(scrollArea);

    // Status
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(statusLabel);

    // Connects
    connect(btnCapture, &QPushButton::clicked,
            this, &ScreenshotPage::onCaptureClicked);
    connect(btnStartLive, &QPushButton::clicked,
            this, &ScreenshotPage::onStartLiveClicked);
    connect(btnStopLive, &QPushButton::clicked,
            this, &ScreenshotPage::onStopLiveClicked);
    connect(btnSave, &QPushButton::clicked,
            this, &ScreenshotPage::onSaveClicked);
}


void ScreenshotPage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers
//---------------------------------------------------

void ScreenshotPage::onCaptureClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Capturing screenshot...");
    Packet packet(Protocol::SCREENSHOT, "");
    clientSocket->sendPacket(packet);
}

void ScreenshotPage::onStartLiveClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Starting live screen...");
    Packet packet(Protocol::START_LIVE_SCREEN, "");
    clientSocket->sendPacket(packet);
}

void ScreenshotPage::onStopLiveClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Stopping live screen...");
    Packet packet(Protocol::STOP_LIVE_SCREEN, "");
    clientSocket->sendPacket(packet);
}

void ScreenshotPage::onSaveClicked()
{
    if (currentPixmap.isNull())
    {
        QMessageBox::warning(this, "Warning",
                             "No screenshot to save!");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Screenshot", "screenshot.png",
        "Images (*.png *.jpg *.bmp)");

    if (!fileName.isEmpty())
    {
        currentPixmap.save(fileName);
        statusLabel->setText("Saved: " + fileName);
    }
}


//---------------------------------------------------
// Hiển thị ảnh từ base64
//---------------------------------------------------

void ScreenshotPage::displayImage(const QString& base64Data)
{
    QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

    QPixmap pixmap;
    pixmap.loadFromData(imageData, "JPEG");

    if (!pixmap.isNull())
    {
        currentPixmap = pixmap;

        // Scale ảnh vừa với label
        QPixmap scaled = pixmap.scaled(
            imageLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        imageLabel->setPixmap(scaled);
    }
}


//---------------------------------------------------
// Xử lý response từ Server
//---------------------------------------------------

void ScreenshotPage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (command == Protocol::SCREENSHOT)
    {
        displayImage(data);
        statusLabel->setText("Screenshot received!");
    }
    else if (command == Protocol::START_LIVE_SCREEN)
    {
        if (data == "SUCCESS")
        {
            statusLabel->setText("Live screen started!");
        }
        else
        {
            // Đây là frame từ live screen
            displayImage(data);
        }
    }
    else if (command == Protocol::STOP_LIVE_SCREEN)
    {
        statusLabel->setText("Live screen stopped.");
    }
}
