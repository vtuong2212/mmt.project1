#include <QApplication>
#include <QDebug>

#include "GUI/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Thiết lập thông tin ứng dụng
    QApplication::setApplicationName("Remote Control Client");
    QApplication::setOrganizationName("RemoteControl");

    MainWindow mainWindow;
    mainWindow.setWindowTitle("Remote Control - Client");
    mainWindow.resize(1000, 700);
    mainWindow.show();

    qDebug() << "Client GUI started.";

    return a.exec();
}
