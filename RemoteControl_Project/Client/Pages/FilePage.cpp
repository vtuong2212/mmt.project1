#include "FilePage.h"
#include "../Network/ClientSocket.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFile>

FilePage::FilePage(QWidget *parent)
    : QWidget(parent), clientSocket(nullptr)
{
    setupUI();
}


//---------------------------------------------------
// Thiết lập giao diện
//---------------------------------------------------

void FilePage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("File Explorer & Download");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // Path input
    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel("Path:");
    pathInput = new QLineEdit();
    pathInput->setPlaceholderText("Enter directory path (e.g., C:/)");
    pathInput->setText("C:/");

    btnGoUp = new QPushButton("↑ Up");
    btnBrowse = new QPushButton("Browse");
    btnDownload = new QPushButton("Download File");

    btnGoUp->setMinimumHeight(35);
    btnBrowse->setMinimumHeight(35);
    btnDownload->setMinimumHeight(35);

    btnDownload->setStyleSheet("background-color: #2980b9; color: white;");

    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathInput, 1);
    pathLayout->addWidget(btnGoUp);
    pathLayout->addWidget(btnBrowse);
    pathLayout->addWidget(btnDownload);

    mainLayout->addLayout(pathLayout);

    // Table
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"Type", "Name", "Size", "Last Modified"});
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setColumnWidth(0, 60);
    tableWidget->setColumnWidth(2, 100);
    tableWidget->setColumnWidth(3, 160);

    mainLayout->addWidget(tableWidget);

    // Progress bar
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    // Status
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(statusLabel);

    // Connects
    connect(btnBrowse, &QPushButton::clicked,
            this, &FilePage::onBrowseClicked);
    connect(btnDownload, &QPushButton::clicked,
            this, &FilePage::onDownloadClicked);
    connect(btnGoUp, &QPushButton::clicked,
            this, &FilePage::onGoUpClicked);
    connect(tableWidget, &QTableWidget::cellDoubleClicked,
            this, &FilePage::onTableDoubleClicked);
}


void FilePage::setClientSocket(ClientSocket* socket)
{
    clientSocket = socket;
}


//---------------------------------------------------
// Button handlers
//---------------------------------------------------

void FilePage::onBrowseClicked()
{
    if (!clientSocket) return;

    QString path = pathInput->text().trimmed();
    statusLabel->setText("Listing files: " + path);
    Packet packet(Protocol::LIST_FILES, path);
    clientSocket->sendPacket(packet);
}

void FilePage::onDownloadClicked()
{
    if (!clientSocket) return;

    int row = tableWidget->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Warning",
                             "Please select a file to download!");
        return;
    }

    QString type = tableWidget->item(row, 0)->text();
    if (type == "DIR")
    {
        QMessageBox::warning(this, "Warning",
                             "Cannot download a directory. Double-click to browse.");
        return;
    }

    QString fileName = tableWidget->item(row, 1)->text();
    QString remotePath = currentPath + "/" + fileName;

    statusLabel->setText("Downloading: " + fileName);
    progressBar->setVisible(true);
    progressBar->setRange(0, 0);  // Indeterminate

    Packet packet(Protocol::DOWNLOAD_FILE, remotePath);
    clientSocket->sendPacket(packet);
}

void FilePage::onGoUpClicked()
{
    if (currentPath.isEmpty()) return;

    QDir dir(currentPath);
    if (dir.cdUp())
    {
        pathInput->setText(dir.absolutePath());
        onBrowseClicked();
    }
}

void FilePage::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column)

    QString type = tableWidget->item(row, 0)->text();
    QString name = tableWidget->item(row, 1)->text();

    if (type == "DIR")
    {
        // Navigate vào thư mục
        QString newPath = currentPath + "/" + name;
        pathInput->setText(newPath);
        onBrowseClicked();
    }
}


//---------------------------------------------------
// Xử lý response từ Server
//---------------------------------------------------

void FilePage::handleResponse(const Packet& packet)
{
    QString command = packet.getCommand();
    QString data = packet.getData();

    if (command == Protocol::LIST_FILES)
    {
        if (data.startsWith("ERROR:"))
        {
            statusLabel->setText(data);
            return;
        }

        QStringList lines = data.split("\n", Qt::SkipEmptyParts);
        tableWidget->setRowCount(0);

        for (const QString& line : lines)
        {
            // Dòng đầu tiên là PATH:
            if (line.startsWith("PATH:"))
            {
                currentPath = line.mid(5);
                pathInput->setText(currentPath);
                continue;
            }

            // Format: TYPE|NAME|SIZE|LAST_MODIFIED
            QStringList parts = line.split("|");
            if (parts.size() >= 4)
            {
                int row = tableWidget->rowCount();
                tableWidget->insertRow(row);

                QString type = parts[0];
                QTableWidgetItem* typeItem = new QTableWidgetItem(type);

                // Icon cho folder/file
                if (type == "DIR")
                {
                    typeItem->setForeground(Qt::blue);
                }

                tableWidget->setItem(row, 0, typeItem);
                tableWidget->setItem(row, 1,
                    new QTableWidgetItem(parts[1]));

                // Format size
                QString sizeStr = parts[2];
                if (!sizeStr.isEmpty())
                {
                    qint64 size = sizeStr.toLongLong();
                    if (size >= 1024 * 1024)
                    {
                        sizeStr = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
                    }
                    else if (size >= 1024)
                    {
                        sizeStr = QString::number(size / 1024.0, 'f', 1) + " KB";
                    }
                    else
                    {
                        sizeStr = QString::number(size) + " B";
                    }
                }

                tableWidget->setItem(row, 2,
                    new QTableWidgetItem(sizeStr));
                tableWidget->setItem(row, 3,
                    new QTableWidgetItem(parts[3]));
            }
        }

        statusLabel->setText("Listed " +
            QString::number(tableWidget->rowCount()) + " items in " + currentPath);
    }
    else if (command == Protocol::DOWNLOAD_FILE)
    {
        progressBar->setVisible(false);

        if (data.startsWith("ERROR:"))
        {
            statusLabel->setText(data);
            return;
        }

        // Format: FILENAME:base64data
        int colonPos = data.indexOf(':');
        if (colonPos > 0)
        {
            QString fileName = data.left(colonPos);
            QString base64Data = data.mid(colonPos + 1);

            // Hỏi user lưu ở đâu
            QString savePath = QFileDialog::getSaveFileName(
                this, "Save File", fileName);

            if (!savePath.isEmpty())
            {
                QByteArray fileData = QByteArray::fromBase64(base64Data.toUtf8());

                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(fileData);
                    file.close();

                    statusLabel->setText("Downloaded: " + fileName +
                        " (" + QString::number(fileData.size()) + " bytes)");
                }
                else
                {
                    statusLabel->setText("Failed to save file!");
                }
            }
        }
    }
}
