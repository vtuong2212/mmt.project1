#include "PowerPage.h"
#include "../Network/ClientSocket.h"

#include <QMessageBox>
#include <QSpacerItem>

PowerPage::PowerPage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void PowerPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Power Control");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Warning
    QLabel* warningLabel = new QLabel(
        "⚠ Warning: These actions will affect the remote computer immediately!");
    warningLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    warningLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(warningLabel);

    mainLayout->addSpacerItem(
        new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Buttons Grid
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(20);

    // Shutdown button
    btnShutdown = new QPushButton("⏻ SHUTDOWN");
    btnShutdown->setMinimumSize(200, 80);
    btnShutdown->setStyleSheet(
        "QPushButton {"
        "  background-color: #e74c3c;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border-radius: 10px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #a93226;"
        "}"
    );

    // Restart button
    btnRestart = new QPushButton("🔄 RESTART");
    btnRestart->setMinimumSize(200, 80);
    btnRestart->setStyleSheet(
        "QPushButton {"
        "  background-color: #e67e22;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border-radius: 10px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #d35400;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #ba4a00;"
        "}"
    );

    // Sleep button
    btnSleep = new QPushButton("🌙 SLEEP");
    btnSleep->setMinimumSize(200, 80);
    btnSleep->setStyleSheet(
        "QPushButton {"
        "  background-color: #2980b9;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border-radius: 10px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2471a3;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1a5276;"
        "}"
    );

    // Log Off button
    btnLogOff = new QPushButton("🚪 LOG OFF");
    btnLogOff->setMinimumSize(200, 80);
    btnLogOff->setStyleSheet(
        "QPushButton {"
        "  background-color: #8e44ad;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border-radius: 10px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #7d3c98;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #6c3483;"
        "}"
    );

    gridLayout->addWidget(btnShutdown, 0, 0);
    gridLayout->addWidget(btnRestart, 0, 1);
    gridLayout->addWidget(btnSleep, 1, 0);
    gridLayout->addWidget(btnLogOff, 1, 1);

    mainLayout->addLayout(gridLayout);

    mainLayout->addSpacerItem(
        new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Status
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: gray;");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // Connects
    connect(btnShutdown, &QPushButton::clicked,
            this, &PowerPage::onShutdownClicked);
    connect(btnRestart, &QPushButton::clicked,
            this, &PowerPage::onRestartClicked);
    connect(btnSleep, &QPushButton::clicked,
            this, &PowerPage::onSleepClicked);
    connect(btnLogOff, &QPushButton::clicked,
            this, &PowerPage::onLogOffClicked);
}


void PowerPage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers (tất cả đều có confirm dialog)
//---------------------------------------------------

void PowerPage::onShutdownClicked()
{
    if (!clientSocket) return;

    int ret = QMessageBox::warning(this, "Confirm Shutdown",
        "Are you sure you want to SHUTDOWN the remote computer?\n"
        "This action cannot be undone!",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        statusLabel->setText("Sending SHUTDOWN command...");
        Packet packet(Protocol::SHUTDOWN, "");
        clientSocket->sendPacket(packet);
    }
}

void PowerPage::onRestartClicked()
{
    if (!clientSocket) return;

    int ret = QMessageBox::warning(this, "Confirm Restart",
        "Are you sure you want to RESTART the remote computer?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        statusLabel->setText("Sending RESTART command...");
        Packet packet(Protocol::RESTART, "");
        clientSocket->sendPacket(packet);
    }
}

void PowerPage::onSleepClicked()
{
    if (!clientSocket) return;

    int ret = QMessageBox::question(this, "Confirm Sleep",
        "Put the remote computer to SLEEP?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        statusLabel->setText("Sending SLEEP command...");
        Packet packet(Protocol::SLEEP, "");
        clientSocket->sendPacket(packet);
    }
}

void PowerPage::onLogOffClicked()
{
    if (!clientSocket) return;

    int ret = QMessageBox::warning(this, "Confirm Log Off",
        "Are you sure you want to LOG OFF the remote computer?\n"
        "All unsaved work will be lost!",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        statusLabel->setText("Sending LOG OFF command...");
        Packet packet(Protocol::LOG_OFF, "");
        clientSocket->sendPacket(packet);
    }
}


//---------------------------------------------------
// Xử lý response từ Server
//---------------------------------------------------

void PowerPage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (data.startsWith("SUCCESS"))
    {
        statusLabel->setText(command + " command sent successfully!");
    }
    else
    {
        statusLabel->setText("Failed: " + data);
    }
}
