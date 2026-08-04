#ifndef PROCESSPAGE_H
#define PROCESSPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

class ClientSocket;

class ProcessPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessPage(QWidget *parent = nullptr);

    void setClientSocket(ClientSocket* socket);
    void handleResponse(const Packet& packet);

private slots:
    void onListClicked();
    void onKillClicked();
    void onStartClicked();

private:
    void setupUI();

    QTableWidget* tableWidget;
    QLineEdit* inputProcess;
    QPushButton* btnList;
    QPushButton* btnKill;
    QPushButton* btnStart;
    QLabel* statusLabel;

    ClientSocket* clientSocket;
};

#endif // PROCESSPAGE_H
