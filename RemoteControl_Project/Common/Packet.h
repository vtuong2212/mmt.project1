#ifndef PACKET_H
#define PACKET_H

#include <QString>

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

private:
    QString command;
    QString data;
};

#endif // PACKET_H
