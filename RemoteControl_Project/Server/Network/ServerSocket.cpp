#include "ServerSocket.h"

#include <QHostAddress>
#include <QDebug>

ServerSocket::ServerSocket(QObject *parent)
    : QObject(parent)
{
    server = new QTcpServer(this);
    clientSocket = nullptr;
}

ServerSocket::~ServerSocket()
{
    stopServer();
}


//---------------------------------------------------
// Khởi động Server
//---------------------------------------------------

bool ServerSocket::startServer(int port)
{
    if (server->listen(QHostAddress::Any, port))
    {
        qDebug() << "Server is running on port:" << port;

        // Chấp nhận Client đầu tiên kết nối tới
        connect(server,
                &QTcpServer::newConnection,
                this,
                [this]()
                {
                    clientSocket = server->nextPendingConnection();

                    qDebug() << "Client connected!";
                });

        return true;
    }

    qDebug() << "Failed to start Server!";
    return false;
}


//---------------------------------------------------
// Dừng Server
//---------------------------------------------------

void ServerSocket::stopServer()
{
    if (clientSocket)
    {
        clientSocket->close();
    }

    if (server->isListening())
    {
        server->close();
    }
}


//---------------------------------------------------
// Gửi Message cho Client
//---------------------------------------------------

void ServerSocket::sendMessage(const QString& message)
{
    if (!isClientConnected())
    {
        qDebug() << "No Client connected!";
        return;
    }

    clientSocket->write(message.toUtf8());
    clientSocket->flush();
}


//---------------------------------------------------
// Gửi Packet cho Client
//---------------------------------------------------

void ServerSocket::sendPacket(const Packet& packet)
{
    QString dataToSend =
            packet.getCommand() +
            "|" +
            packet.getData();

    sendMessage(dataToSend);
}


//---------------------------------------------------
// Kiểm tra trạng thái kết nối
//---------------------------------------------------

bool ServerSocket::isClientConnected() const
{
    if (clientSocket == nullptr)
    {
        return false;
    }

    return clientSocket->state()
            == QAbstractSocket::ConnectedState;
}
