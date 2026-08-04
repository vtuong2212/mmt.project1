#ifndef WEBCAMPAGE_H
#define WEBCAMPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

class ClientSocket;

class WebcamPage : public QWidget
{
    Q_OBJECT

public:
    explicit WebcamPage(QWidget *parent = nullptr);

    void setClientSocket(ClientSocket* socket);
    void handleResponse(const Packet& packet);

private slots:
    void onCaptureClicked();
    void onStartStreamClicked();
    void onStopStreamClicked();
    void onSaveClicked();

private:
    void setupUI();
    void displayImage(const QString& base64Data);

    QLabel* imageLabel;
    QPushButton* btnCapture;
    QPushButton* btnStartStream;
    QPushButton* btnStopStream;
    QPushButton* btnSave;
    QLabel* statusLabel;
    QScrollArea* scrollArea;

    QPixmap currentPixmap;
    ClientSocket* clientSocket;
};

#endif // WEBCAMPAGE_H
