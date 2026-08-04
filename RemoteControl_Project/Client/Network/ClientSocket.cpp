#include "ClientSocket.h"

#include <QHostAddress>
#include <QDataStream>
#include <QDebug>

ClientSocket::ClientSocket(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);

    // Kết nối signals của QTcpSocket
    connect(socket,
            &QTcpSocket::readyRead,
            this,
            &ClientSocket::onReadyRead);

    connect(socket,
            &QTcpSocket::connected,
            this,
            &ClientSocket::connected);

    connect(socket,
            &QTcpSocket::disconnected,
            this,
            &ClientSocket::disconnected);

    connect(socket,
            &QAbstractSocket::errorOccurred,
            this,
            &ClientSocket::onErrorOccurred);
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
    buffer.clear();

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
    buffer.clear();
    socket->disconnectFromHost();

    qDebug() << "Disconnected from Server!";
}


//---------------------------------------------------
// Đọc dữ liệu từ Server
// Protocol: [4 bytes size][serialized packet data]
//---------------------------------------------------

void ClientSocket::onReadyRead()
{
    buffer.append(socket->readAll());

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

        qDebug() << "Received response:"
                 << packet.getCommand();

        emit packetReceived(packet);
    }
}


//---------------------------------------------------
// Xử lý lỗi kết nối
//---------------------------------------------------

void ClientSocket::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)

    QString errorMsg = socket->errorString();
    qDebug() << "Socket error:" << errorMsg;

    emit errorOccurred(errorMsg);
}


//---------------------------------------------------
// Gửi chuỗi dữ liệu (raw)
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
// Gửi Packet (có size header)
//---------------------------------------------------

void ClientSocket::sendPacket(const Packet& packet)
{
    if (!isConnected())
    {
        qDebug() << "No connection!";
        return;
    }

    QByteArray serialized = packet.serialize();
    socket->write(serialized);
    socket->flush();
}


//---------------------------------------------------
// Kiểm tra trạng thái kết nối
//---------------------------------------------------

bool ClientSocket::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}
