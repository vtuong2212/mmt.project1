#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:

    Ui::MainWindow *ui;

    ApplicationPage *applicationPage;
    ProcessPage *processPage;
    ScreenshotPage *screenshotPage;
    KeyloggerPage *keyloggerPage;
    FilePage *filePage;
    WebcamPage *webcamPage;
    PowerPage *powerPage;
};

#endif // MAINWINDOW_H
