#ifndef CLIENTSOCKET_CPP
#define CLIENTSOCKET_CPP

#include "ClientSocket.h"

#include <QHostAddress>
#include <QByteArray>
#include <QDebug>

ClientSocket::ClientSocket(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);
}

ClientSocket::~ClientSocket()
{
    if (socket->isOpen())
    {
        socket->close();
    }
}


//---------------------------------------------------
// Kết nối tới Server
//---------------------------------------------------

bool ClientSocket::connectToServer(const QString& ipAddress,
                                   int port)
{
    socket->connectToHost(ipAddress, port);

    if (socket->waitForConnected(Constants::CONNECTION_TIMEOUT))
    {
        qDebug() << "Connected to Server!";
        return true;
    }

    qDebug() << "Failed to connect!";
    return false;
}


//---------------------------------------------------
// Ngắt kết nối
//---------------------------------------------------

void ClientSocket::disconnectFromServer()
{
    socket->disconnectFromHost();

    qDebug() << "Disconnected from Server!";
}


//---------------------------------------------------
// Gửi chuỗi dữ liệu
//---------------------------------------------------

void ClientSocket::sendMessage(const QString& message)
{
    if (!isConnected())
    {
        qDebug() << "No connection!";
        return;
    }

    socket->write(message.toUtf8());
    socket->flush();
}


//---------------------------------------------------
// Gửi Packet
//---------------------------------------------------

void ClientSocket::sendPacket(const Packet& packet)
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

bool ClientSocket::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

#endif
