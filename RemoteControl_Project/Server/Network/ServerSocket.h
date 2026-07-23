#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#include "../../Common/Packet.h"
#include "../../Common/Constants.h"
#include "../../Common/Protocol.h"

class ServerSocket : public QObject
{
    Q_OBJECT

public:
    explicit ServerSocket(QObject *parent = nullptr);
    ~ServerSocket();

    // Khởi động Server
    bool startServer(int port = Constants::SERVER_PORT);

    // Dừng Server
    void stopServer();

    // Gửi dữ liệu cho Client
    void sendMessage(const QString& message);

    // Gửi Packet cho Client
    void sendPacket(const Packet& packet);

    // Kiểm tra Client có đang kết nối hay không
    bool isClientConnected() const;

private:
    QTcpServer* server;
    QTcpSocket* clientSocket;
};

#endif // SERVERSOCKET_H
