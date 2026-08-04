#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>

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

    // Gửi Packet cho Client (có size header)
    void sendPacket(const Packet& packet);

    // Kiểm tra Client có đang kết nối hay không
    bool isClientConnected() const;

signals:
    // Signal phát khi nhận được Packet từ Client
    void packetReceived(const Packet& packet);

    // Signal khi Client kết nối/ngắt kết nối
    void clientConnected();
    void clientDisconnected();

private slots:
    // Xử lý Client mới kết nối
    void onNewConnection();

    // Đọc dữ liệu từ Client
    void onReadyRead();

    // Xử lý khi Client ngắt kết nối
    void onClientDisconnected();

private:
    QTcpServer* server;
    QTcpSocket* clientSocket;
    QByteArray buffer;  // Buffer tích lũy dữ liệu
};

#endif // SERVERSOCKET_H
