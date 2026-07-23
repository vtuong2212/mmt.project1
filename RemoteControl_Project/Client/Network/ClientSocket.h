#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

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

    // Gửi dữ liệu
    void sendMessage(const QString& message);

    // Gửi Packet
    void sendPacket(const Packet& packet);

    // Kiểm tra trạng thái kết nối
    bool isConnected() const;

private:
    QTcpSocket* socket;
};

#endif // CLIENTSOCKET_H
