#ifndef PACKET_H
#define PACKET_H

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

class Packet
{
public:
    // Constructor mặc định
    Packet() = default;

    // Constructor có tham số
    Packet(const QString& cmd, const QString& msg)
        : command(cmd), data(msg)
    {
    }

    // Getter
    QString getCommand() const
    {
        return command;
    }

    QString getData() const
    {
        return data;
    }

    // Setter
    void setCommand(const QString& cmd)
    {
        command = cmd;
    }

    void setData(const QString& msg)
    {
        data = msg;
    }

    //=========================================
    // Serialize: Packet → QByteArray
    // Format: [4 bytes size][command|data]
    //=========================================

    QByteArray serialize() const
    {
        QByteArray body;
        QDataStream stream(&body, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_6_0);

        stream << command << data;

        // Tạo header chứa kích thước body
        QByteArray packet;
        QDataStream headerStream(&packet, QIODevice::WriteOnly);
        headerStream.setVersion(QDataStream::Qt_6_0);

        headerStream << (quint32)body.size();
        packet.append(body);

        return packet;
    }

    //=========================================
    // Deserialize: QByteArray → Packet
    // Giả sử body đã được tách ra (không có header)
    //=========================================

    static Packet deserialize(const QByteArray& body)
    {
        Packet pkt;
        QDataStream stream(body);
        stream.setVersion(QDataStream::Qt_6_0);

        stream >> pkt.command >> pkt.data;

        return pkt;
    }

private:
    QString command;
    QString data;
};

#endif // PACKET_H
