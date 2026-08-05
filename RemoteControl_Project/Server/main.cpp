#include <QGuiApplication>
#include <QDebug>

#include "Network/ServerSocket.h"
#include "Command/CommandManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    qDebug() << "========================================";
    qDebug() << " Remote Control Server";
    qDebug() << "========================================";

    ServerSocket server;
    CommandManager commandManager;

    //---------------------------------------------------
    // Kết nối: Client gửi packet → CommandManager xử lý
    //---------------------------------------------------

    QObject::connect(&server,
                     &ServerSocket::packetReceived,
                     &commandManager,
                     &CommandManager::handleCommand);

    //---------------------------------------------------
    // Kết nối: CommandManager trả response → Server gửi lại Client
    //---------------------------------------------------

    QObject::connect(&commandManager,
                     &CommandManager::responseReady,
                     &server,
                     &ServerSocket::sendPacket);

    //---------------------------------------------------
    // Log kết nối
    //---------------------------------------------------

    QObject::connect(&server,
                     &ServerSocket::clientConnected,
                     []()
    {
        qDebug() << ">>> Client has connected!";
    });

    QObject::connect(&server,
                     &ServerSocket::clientDisconnected,
                     []()
    {
        qDebug() << ">>> Client has disconnected!";
    });

    //---------------------------------------------------
    // Khởi động Server
    //---------------------------------------------------

    if (server.startServer())
    {
        qDebug() << "Server started successfully!";
        qDebug() << "Waiting for Client connection...";
    }
    else
    {
        qDebug() << "Failed to start server!";
        return -1;
    }

    return a.exec();
}
