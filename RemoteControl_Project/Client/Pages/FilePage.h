#ifndef FILEPATH_H
#define FILEPATH_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

class ClientSocket;

class FilePage : public QWidget
{
    Q_OBJECT

public:
    explicit FilePage(QWidget *parent = nullptr);

    void setClientSocket(ClientSocket* socket);
    void handleResponse(const Packet& packet);

private slots:
    void onBrowseClicked();
    void onDownloadClicked();
    void onGoUpClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();

    QLineEdit* pathInput;
    QTableWidget* tableWidget;
    QPushButton* btnBrowse;
    QPushButton* btnDownload;
    QPushButton* btnGoUp;
    QLabel* statusLabel;
    QProgressBar* progressBar;

    QString currentPath;
    ClientSocket* clientSocket;
};

#endif // FILEPATH_H
