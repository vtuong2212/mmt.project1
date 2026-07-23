#include <QCoreApplication>
#include <QDebug>

#include "Network/ServerSocket.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ServerSocket server;

    if(server.startServer())
    {
        qDebug() << "Server started successfully!";
    }
    else
    {
        qDebug() << "Failed to start server!";
    }

    return a.exec();
}
