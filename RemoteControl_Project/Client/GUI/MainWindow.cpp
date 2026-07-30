#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QPushButton>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //-----------------------------------
    // Create Pages
    //-----------------------------------

    applicationPage = new ApplicationPage(this);
    processPage = new ProcessPage(this);
    screenshotPage = new ScreenshotPage(this);
    keyloggerPage = new KeyloggerPage(this);
    filePage = new FilePage(this);
    webcamPage = new WebcamPage(this);
    powerPage = new PowerPage(this);


    //-----------------------------------
    // Add Pages to TabWidget
    //-----------------------------------

    ui->tabWidget->addTab(applicationPage,
                          "Application");

    ui->tabWidget->addTab(processPage,
                          "Processes");

    ui->tabWidget->addTab(screenshotPage,
                          "Screenshot");

    ui->tabWidget->addTab(keyloggerPage,
                          "Keylogger");

    ui->tabWidget->addTab(filePage,
                          "File Download");

    ui->tabWidget->addTab(webcamPage,
                          "Webcam");

    ui->tabWidget->addTab(powerPage,
                          "Power Control");


    //-----------------------------------
    // Connect Button Signals
    //-----------------------------------

    connect(ui->btnConnect,
            &QPushButton::clicked,
            this,
            &MainWindow::onConnectClicked);



    connect(ui->btnDisconnect,
            &QPushButton::clicked,
            this,
            &MainWindow::onDisconnectClicked);

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onConnectClicked()
{
    qDebug() << "Connect button clicked!";
}


void MainWindow::onDisconnectClicked()
{
    qDebug() << "Disconnect button clicked!";
}
