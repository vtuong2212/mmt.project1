#include <QCoreApplication>
#include <QDebug>

#include "Network/ClientSocket.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ClientSocket client;

    if(client.connectToServer("127.0.0.1"))
    {
        qDebug() << "Connection successful!";
    }
    else
    {
        qDebug() << "Connection failed!";
    }

    return a.exec();
}
