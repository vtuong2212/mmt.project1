// packet.h
#ifndef PACKET_H
#define PACKET_H

#include <QString>

class Packet
{
public:
    Packet();

    Packet(const QString& cmd,
           const QString& msg);

    // Getter
    QString getCommand() const;
    QString getData() const;

    // Setter
    void setCommand(const QString& cmd);
    void setData(const QString& msg);

private:
    QString command;
    QString data;
};

#endif // PACKET_H
