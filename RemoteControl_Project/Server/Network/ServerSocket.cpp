#include "ServerSocket.h"

#include <QHostAddress>
#include <QDataStream>
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

        connect(server,
                &QTcpServer::newConnection,
                this,
                &ServerSocket::onNewConnection);

        return true;
    }

    qDebug() << "Failed to start Server!";
    return false;
}


//---------------------------------------------------
// Xử lý Client mới kết nối
//---------------------------------------------------

void ServerSocket::onNewConnection()
{
    // Cleanup socket cũ nếu còn tồn tại
    if (clientSocket)
    {
        clientSocket->disconnect();  // Ngắt tất cả signals
        clientSocket->close();
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }

    clientSocket = server->nextPendingConnection();

    if (!clientSocket)
    {
        return;
    }

    qDebug() << "Client connected from:"
             << clientSocket->peerAddress().toString()
             << ":" << clientSocket->peerPort();

    // Kết nối signals
    connect(clientSocket,
            &QTcpSocket::readyRead,
            this,
            &ServerSocket::onReadyRead);

    connect(clientSocket,
            &QTcpSocket::disconnected,
            this,
            &ServerSocket::onClientDisconnected);

    buffer.clear();

    emit clientConnected();
}


//---------------------------------------------------
// Đọc dữ liệu từ Client
// Protocol: [4 bytes size][serialized packet data]
//---------------------------------------------------

void ServerSocket::onReadyRead()
{
    buffer.append(clientSocket->readAll());

    // Xử lý tất cả packet trong buffer
    while (buffer.size() >= 4)
    {
        // Đọc kích thước body
        QDataStream sizeStream(buffer.left(4));
        sizeStream.setVersion(QDataStream::Qt_6_0);

        quint32 bodySize;
        sizeStream >> bodySize;

        // Kiểm tra đã nhận đủ data chưa
        if ((quint32)buffer.size() < 4 + bodySize)
        {
            // Chưa đủ, chờ thêm data
            break;
        }

        // Tách body ra
        QByteArray body = buffer.mid(4, bodySize);
        buffer.remove(0, 4 + bodySize);

        // Deserialize thành Packet
        Packet packet = Packet::deserialize(body);

        qDebug() << "Received packet:"
                 << packet.getCommand();

        emit packetReceived(packet);
    }
}


//---------------------------------------------------
// Xử lý khi Client ngắt kết nối
//---------------------------------------------------

void ServerSocket::onClientDisconnected()
{
    qDebug() << "Client disconnected!";

    buffer.clear();

    // Cleanup socket để tránh dangling pointer
    if (clientSocket)
    {
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }

    emit clientDisconnected();
}


//---------------------------------------------------
// Dừng Server
//---------------------------------------------------

void ServerSocket::stopServer()
{
    if (clientSocket)
    {
        clientSocket->disconnect();  // Ngắt signals trước
        clientSocket->close();
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }

    if (server && server->isListening())
    {
        server->close();
    }
}


//---------------------------------------------------
// Gửi Message cho Client (raw string)
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
// Gửi Packet cho Client (có size header)
//---------------------------------------------------

void ServerSocket::sendPacket(const Packet& packet)
{
    if (!isClientConnected())
    {
        qDebug() << "No Client connected!";
        return;
    }

    QByteArray serialized = packet.serialize();
    clientSocket->write(serialized);
    clientSocket->flush();
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
