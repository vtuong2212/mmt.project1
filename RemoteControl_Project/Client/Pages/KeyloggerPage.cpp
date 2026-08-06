#include "KeyloggerPage.h"
#include "../Network/ClientSocket.h"

#include <QTextCursor>

KeyloggerPage::KeyloggerPage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void KeyloggerPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Keylogger");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    btnStart = new QPushButton("Start Keylogger");
    btnStop = new QPushButton("Stop Keylogger");
    btnGetData = new QPushButton("Get Keylog Data");
    btnClear = new QPushButton("Clear Display");

    btnStart->setMinimumHeight(35);
    btnStop->setMinimumHeight(35);
    btnGetData->setMinimumHeight(35);
    btnClear->setMinimumHeight(35);

    btnStart->setStyleSheet("background-color: #27ae60; color: white;");
    btnStop->setStyleSheet("background-color: #e74c3c; color: white;");

    buttonLayout->addWidget(btnStart);
    buttonLayout->addWidget(btnStop);
    buttonLayout->addWidget(btnGetData);
    buttonLayout->addWidget(btnClear);

    mainLayout->addLayout(buttonLayout);

    // Text display
    textDisplay = new QTextEdit();
    textDisplay->setReadOnly(true);
    textDisplay->setStyleSheet("background-color: #1e1e1e; color: #00ff00; "
                               "font-family: Consolas, monospace; font-size: 13px;");
    textDisplay->setPlaceholderText("Keylogger data will appear here...");

    mainLayout->addWidget(textDisplay);

    // Status
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(statusLabel);

    // Connects
    connect(btnStart, &QPushButton::clicked,
            this, &KeyloggerPage::onStartClicked);
    connect(btnStop, &QPushButton::clicked,
            this, &KeyloggerPage::onStopClicked);
    connect(btnGetData, &QPushButton::clicked,
            this, &KeyloggerPage::onGetDataClicked);
    connect(btnClear, &QPushButton::clicked,
            this, &KeyloggerPage::onClearClicked);
}


void KeyloggerPage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers
//---------------------------------------------------

void KeyloggerPage::onStartClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Starting keylogger...");
    Packet packet(Protocol::START_KEYLOGGER, "");
    clientSocket->sendPacket(packet);
}

void KeyloggerPage::onStopClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Stopping keylogger...");
    Packet packet(Protocol::STOP_KEYLOGGER, "");
    clientSocket->sendPacket(packet);
}

void KeyloggerPage::onGetDataClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Requesting keylog data...");
    Packet packet(Protocol::GET_KEYLOGGER_DATA, "");
    clientSocket->sendPacket(packet);
}

void KeyloggerPage::onClearClicked()
{
    textDisplay->clear();
    statusLabel->setText("Display cleared.");
}


//---------------------------------------------------
// Xử lý response từ Server
//---------------------------------------------------

void KeyloggerPage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (command == Protocol::START_KEYLOGGER)
    {
        if (data.startsWith("SUCCESS"))
        {
            statusLabel->setText("Keylogger started!");
        }
        else
        {
            statusLabel->setText("Failed: " + data);
        }
    }
    else if (command == Protocol::STOP_KEYLOGGER)
    {
        if (data.startsWith("SUCCESS"))
        {
            statusLabel->setText("Keylogger stopped!");
        }
        else
        {
            statusLabel->setText("Failed: " + data);
        }
    }
    else if (command == Protocol::GET_KEYLOGGER_DATA)
    {
        // Real-time: xử lý từng ký tự, bao gồm backspace từ IME
        textDisplay->moveCursor(QTextCursor::End);

        for (int i = 0; i < data.length(); i++)
        {
            if (data[i] == '\b')
            {
                // Backspace từ IME (Unikey/Telex xóa ký tự cũ)
                QTextCursor cursor = textDisplay->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.deletePreviousChar();
                textDisplay->setTextCursor(cursor);
            }
            else
            {
                textDisplay->insertPlainText(QString(data[i]));
            }
        }

        textDisplay->moveCursor(QTextCursor::End);
    }
}
