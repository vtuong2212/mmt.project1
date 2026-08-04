#ifndef POWERPAGE_H
#define POWERPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

class ClientSocket;

class PowerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PowerPage(QWidget *parent = nullptr);

    void setClientSocket(ClientSocket* socket);
    void handleResponse(const Packet& packet);

private slots:
    void onShutdownClicked();
    void onRestartClicked();
    void onSleepClicked();
    void onLogOffClicked();

private:
    void setupUI();

    QPushButton* btnShutdown;
    QPushButton* btnRestart;
    QPushButton* btnSleep;
    QPushButton* btnLogOff;
    QLabel* statusLabel;

    ClientSocket* clientSocket;
};

#endif // POWERPAGE_H
