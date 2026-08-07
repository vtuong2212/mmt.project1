#include "WebcamPage.h"
#include "../Network/ClientSocket.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QByteArray>

WebcamPage::WebcamPage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void WebcamPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Webcam Control");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    btnCapture = new QPushButton("Capture Webcam");
    btnStartStream = new QPushButton("Start Stream");
    btnStopStream = new QPushButton("Stop Stream");
    btnSave = new QPushButton("Save Image");

    btnCapture->setMinimumHeight(35);
    btnStartStream->setMinimumHeight(35);
    btnStopStream->setMinimumHeight(35);
    btnSave->setMinimumHeight(35);

    btnStartStream->setStyleSheet("background-color: #27ae60; color: white;");
    btnStopStream->setStyleSheet("background-color: #e74c3c; color: white;");

    buttonLayout->addWidget(btnCapture);
    buttonLayout->addWidget(btnStartStream);
    buttonLayout->addWidget(btnStopStream);
    buttonLayout->addWidget(btnSave);

    mainLayout->addLayout(buttonLayout);

    // Image display
    scrollArea = new QScrollArea();
    imageLabel = new QLabel("No webcam image yet");
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
            this, &WebcamPage::onCaptureClicked);
    connect(btnStartStream, &QPushButton::clicked,
            this, &WebcamPage::onStartStreamClicked);
    connect(btnStopStream, &QPushButton::clicked,
            this, &WebcamPage::onStopStreamClicked);
    connect(btnSave, &QPushButton::clicked,
            this, &WebcamPage::onSaveClicked);
}


void WebcamPage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers
//---------------------------------------------------

void WebcamPage::onCaptureClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Capturing webcam...");
    Packet packet(Protocol::CAPTURE_WEBCAM, "");
    clientSocket->sendPacket(packet);
}

void WebcamPage::onStartStreamClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Starting webcam stream...");
    Packet packet(Protocol::START_WEBCAM_STREAM, "");
    clientSocket->sendPacket(packet);
}

void WebcamPage::onStopStreamClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Stopping webcam stream...");
    Packet packet(Protocol::STOP_WEBCAM_STREAM, "");
    clientSocket->sendPacket(packet);
}

void WebcamPage::onSaveClicked()
{
    if (currentPixmap.isNull())
    {
        QMessageBox::warning(this, "Warning",
                             "No webcam image to save!");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Webcam Image", "webcam.png",
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

void WebcamPage::displayImage(const QString& base64Data)
{
    QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

    QPixmap pixmap;
    pixmap.loadFromData(imageData, "JPEG");

    if (!pixmap.isNull())
    {
        currentPixmap = pixmap;

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

void WebcamPage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (command == Protocol::CAPTURE_WEBCAM)
    {
        if (data.startsWith("ERROR:"))
        {
            statusLabel->setText(data);
        }
        else
        {
            displayImage(data);
            statusLabel->setText("Webcam image received!");
        }
    }
    else if (command == Protocol::START_WEBCAM_STREAM)
    {
        if (data == "SUCCESS")
        {
            statusLabel->setText("Webcam stream started!");
        }
        else if (data.startsWith("ERROR:"))
        {
            statusLabel->setText(data);
        }
        else
        {
            // Frame từ stream
            displayImage(data);
        }
    }
    else if (command == Protocol::STOP_WEBCAM_STREAM)
    {
        statusLabel->setText("Webcam stream stopped.");
    }
}
