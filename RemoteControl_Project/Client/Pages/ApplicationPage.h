#ifndef APPLICATIONPAGE_H
#define APPLICATIONPAGE_H

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

class ApplicationPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplicationPage(QWidget *parent = nullptr);

    // Thiết lập ClientSocket
    void setClientSocket(ClientSocket* socket);

    // Xử lý response từ Server
    void handleResponse(const Packet& packet);

private slots:
    void onListClicked();
    void onOpenClicked();
    void onCloseSelectedClicked();

private:
    void setupUI();

    QTableWidget* tableWidget;
    QLineEdit* inputPath;
    QPushButton* btnList;
    QPushButton* btnOpen;
    QPushButton* btnClose;
    QLabel* statusLabel;

    ClientSocket* clientSocket;
};

#endif // APPLICATIONPAGE_H
