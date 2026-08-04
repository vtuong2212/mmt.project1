#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>

#include "../../Common/Packet.h"
#include "../../Common/Constants.h"
#include "../../Common/Protocol.h"

class ClientSocket : public QObject
{
    Q_OBJECT

public:
    explicit ClientSocket(QObject *parent = nullptr);
    ~ClientSocket();

    // Kết nối tới Server
    bool connectToServer(const QString& ipAddress,
                         int port = Constants::SERVER_PORT);

    // Ngắt kết nối
    void disconnectFromServer();

    // Gửi dữ liệu (raw string)
    void sendMessage(const QString& message);

    // Gửi Packet (có size header)
    void sendPacket(const Packet& packet);

    // Kiểm tra trạng thái kết nối
    bool isConnected() const;

signals:
    // Signal khi nhận được Packet từ Server
    void packetReceived(const Packet& packet);

    // Signal trạng thái kết nối
    void connected();
    void disconnected();
    void errorOccurred(const QString& errorMessage);

private slots:
    // Đọc dữ liệu từ Server
    void onReadyRead();

    // Xử lý lỗi
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* socket;
    QByteArray buffer;  // Buffer tích lũy dữ liệu
};

#endif // CLIENTSOCKET_H
