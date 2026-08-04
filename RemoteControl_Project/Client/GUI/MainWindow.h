#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../Network/ClientSocket.h"

#include "../Pages/ApplicationPage.h"
#include "../Pages/ProcessPage.h"
#include "../Pages/ScreenshotPage.h"
#include "../Pages/KeyloggerPage.h"
#include "../Pages/FilePage.h"
#include "../Pages/WebcamPage.h"
#include "../Pages/PowerPage.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onConnectClicked();
    void onDisconnectClicked();

    // Xử lý tất cả packet nhận từ Server
    void onPacketReceived(const Packet& packet);

    // Xử lý trạng thái kết nối
    void onConnected();
    void onDisconnected();
    void onError(const QString& errorMessage);

private:

    Ui::MainWindow *ui;

    // Network
    ClientSocket* clientSocket;

    // Pages
    ApplicationPage *applicationPage;
    ProcessPage *processPage;
    ScreenshotPage *screenshotPage;
    KeyloggerPage *keyloggerPage;
    FilePage *filePage;
    WebcamPage *webcamPage;
    PowerPage *powerPage;
};

#endif // MAINWINDOW_H
