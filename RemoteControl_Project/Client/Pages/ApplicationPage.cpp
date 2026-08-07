#include "ApplicationPage.h"
#include "../Network/ClientSocket.h"

#include <QHeaderView>
#include <QMessageBox>

ApplicationPage::ApplicationPage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void ApplicationPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Application Management");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // Buttons row
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    btnList = new QPushButton("List Applications");
    btnOpen = new QPushButton("Open Application");
    btnClose = new QPushButton("Close Selected Application");

    btnList->setMinimumHeight(35);
    btnOpen->setMinimumHeight(35);
    btnClose->setMinimumHeight(35);

    buttonLayout->addWidget(btnList);
    buttonLayout->addWidget(btnOpen);
    buttonLayout->addWidget(btnClose);

    mainLayout->addLayout(buttonLayout);

    // Input path (chỉ dùng cho Open Application)
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel("App Path:");
    inputPath = new QLineEdit();
    inputPath->setPlaceholderText("Enter application path to open...");

    inputLayout->addWidget(pathLabel);
    inputLayout->addWidget(inputPath);
    mainLayout->addLayout(inputLayout);

    // Table
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({"Application Name", "Install Location"});
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
            this, &ApplicationPage::onListClicked);
    connect(btnOpen, &QPushButton::clicked,
            this, &ApplicationPage::onOpenClicked);
    connect(btnClose, &QPushButton::clicked,
            this, &ApplicationPage::onCloseSelectedClicked);
}


//---------------------------------------------------
// Set ClientSocket
//---------------------------------------------------

void ApplicationPage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers
//---------------------------------------------------

void ApplicationPage::onListClicked()
{
    if (!clientSocket) return;

    statusLabel->setText("Requesting application list...");
    Packet packet(Protocol::LIST_APPLICATION, "");
    clientSocket->sendPacket(packet);
}

void ApplicationPage::onOpenClicked()
{
    if (!clientSocket) return;

    QString path = inputPath->text().trimmed();
    if (path.isEmpty())
    {
        QMessageBox::warning(this, "Warning",
                             "Please enter application path!");
        return;
    }

    statusLabel->setText("Opening: " + path);
    Packet packet(Protocol::OPEN_APPLICATION, path);
    clientSocket->sendPacket(packet);
}

void ApplicationPage::onCloseSelectedClicked()
{
    if (!clientSocket) return;

    int row = tableWidget->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Warning",
                             "Please select an application first.");
        return;
    }

    QString name = tableWidget->item(row, 0)->text();

    statusLabel->setText("Closing: " + name);
    Packet packet(Protocol::CLOSE_APPLICATION, name);
    clientSocket->sendPacket(packet);
}


//---------------------------------------------------
// Xử lý response từ Server
//---------------------------------------------------

void ApplicationPage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (command == Protocol::LIST_APPLICATION)
    {
        // Parse danh sách ứng dụng
        QStringList apps = data.split("\n", Qt::SkipEmptyParts);
        tableWidget->setRowCount(0);

        for (const QString& app : apps)
        {
            QStringList parts = app.split(" | ");
            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);

            tableWidget->setItem(row, 0,
                                 new QTableWidgetItem(parts.value(0)));
            tableWidget->setItem(row, 1,
                                 new QTableWidgetItem(parts.value(1)));
        }

        statusLabel->setText("Found " + QString::number(apps.size()) + " applications");
    }
    else if (command == Protocol::OPEN_APPLICATION ||
             command == Protocol::CLOSE_APPLICATION)
    {
        if (data.startsWith("SUCCESS"))
        {
            statusLabel->setText("Operation successful!");
        }
        else
        {
            statusLabel->setText("Operation failed: " + data);
        }
    }
}
