#include "ProcessPage.h"
#include "../Network/ClientSocket.h"

#include <QHeaderView>
#include <QMessageBox>

ProcessPage::ProcessPage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void ProcessPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Process Management");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    btnList = new QPushButton("List Processes");
    btnKill = new QPushButton("Kill Process");
    btnStart = new QPushButton("Start Process");

    btnList->setMinimumHeight(35);
    btnKill->setMinimumHeight(35);
    btnStart->setMinimumHeight(35);

    btnKill->setStyleSheet("background-color: #e74c3c; color: white;");

    buttonLayout->addWidget(btnList);
    buttonLayout->addWidget(btnKill);
    buttonLayout->addWidget(btnStart);

    mainLayout->addLayout(buttonLayout);

    // Input
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* inputLabel = new QLabel("Process Path:");
    inputProcess = new QLineEdit();
    inputProcess->setPlaceholderText("Enter process path to start...");

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputProcess);
    mainLayout->addLayout(inputLayout);

    // Table
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({"Process Name", "PID", "Memory"});
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);

    mainLayout->addWidget(tableWidget);

    // Status
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(statusLabel);

    // Connects
    connect(btnList, &QPushButton::clicked,
            this, &ProcessPage::onListClicked);
    connect(btnKill, &QPushButton::clicked,
            this, &ProcessPage::onKillClicked);
    connect(btnStart, &QPushButton::clicked,
            this, &ProcessPage::onStartClicked);
}


void ProcessPage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers
//---------------------------------------------------

void ProcessPage::onListClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Requesting process list...");
    Packet packet(Protocol::LIST_PROCESS, "");
    clientSocket->sendPacket(packet);
}

void ProcessPage::onKillClicked()
{
    if (!clientSocket) return;

    // Lấy PID từ dòng đang chọn
    int row = tableWidget->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Warning",
                             "Please select a process to kill!");
        return;
    }

    QString pid = tableWidget->item(row, 1)->text();
    QString name = tableWidget->item(row, 0)->text();

    int ret = QMessageBox::question(this, "Confirm",
        "Kill process: " + name + " (PID: " + pid + ")?");

    if (ret == QMessageBox::Yes)
    {
        statusLabel->setText("Killing PID: " + pid);
        Packet packet(Protocol::KILL_PROCESS, pid);
        clientSocket->sendPacket(packet);
    }
}

void ProcessPage::onStartClicked()
{
    if (!clientSocket) return;

    QString path = inputProcess->text().trimmed();
    if (path.isEmpty())
    {
        QMessageBox::warning(this, "Warning",
                             "Please enter process path!");
        return;
    }

    statusLabel->setText("Starting: " + path);
    Packet packet(Protocol::START_PROCESS, path);
    clientSocket->sendPacket(packet);
}


//---------------------------------------------------
// Xử lý response từ Server
//---------------------------------------------------

void ProcessPage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (command == Protocol::LIST_PROCESS)
    {
        QStringList processes = data.split("\n", Qt::SkipEmptyParts);
        tableWidget->setRowCount(0);

        for (const QString& proc : processes)
        {
            // Format: Name|PID|Memory
            QStringList parts = proc.split("|");
            if (parts.size() >= 3)
            {
                int row = tableWidget->rowCount();
                tableWidget->insertRow(row);

                tableWidget->setItem(row, 0,
                    new QTableWidgetItem(parts[0].trimmed()));
                tableWidget->setItem(row, 1,
                    new QTableWidgetItem(parts[1].trimmed()));
                tableWidget->setItem(row, 2,
                    new QTableWidgetItem(parts[2].trimmed()));
            }
        }

        statusLabel->setText("Found " +
            QString::number(tableWidget->rowCount()) + " processes");
    }
    else if (command == Protocol::KILL_PROCESS ||
             command == Protocol::START_PROCESS)
    {
        if (data.startsWith("SUCCESS"))
        {
            statusLabel->setText("Operation successful!");
            // Auto-refresh danh sách
            onListClicked();
        }
        else
        {
            statusLabel->setText("Failed: " + data);
        }
    }
}
