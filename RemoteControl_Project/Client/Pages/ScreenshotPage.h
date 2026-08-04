#ifndef SCREENSHOTPAGE_H
#define SCREENSHOTPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

#include "../../Common/Packet.h"
#include "../../Common/Protocol.h"

class ClientSocket;

class ScreenshotPage : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenshotPage(QWidget *parent = nullptr);

    void setClientSocket(ClientSocket* socket);
    void handleResponse(const Packet& packet);

private slots:
    void onCaptureClicked();
    void onStartLiveClicked();
    void onStopLiveClicked();
    void onSaveClicked();

private:
    void setupUI();
    void displayImage(const QString& base64Data);

    QLabel* imageLabel;
    QPushButton* btnCapture;
    QPushButton* btnStartLive;
    QPushButton* btnStopLive;
    QPushButton* btnSave;
    QLabel* statusLabel;
    QScrollArea* scrollArea;

    QPixmap currentPixmap;
    ClientSocket* clientSocket;
};

#endif // SCREENSHOTPAGE_H
