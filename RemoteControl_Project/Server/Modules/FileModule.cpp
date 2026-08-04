#include "FileModule.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

FileModule::FileModule(QObject *parent)
    : QObject(parent)
{
}


//---------------------------------------------------
// Liệt kê file/folder trong thư mục
// Trả về: TYPE|NAME|SIZE cho mỗi entry
//---------------------------------------------------

QString FileModule::listFiles(const QString& directoryPath)
{
    QString path = directoryPath;
    if (path.isEmpty())
    {
        // Mặc định: ổ C:\ trên Windows, / trên Linux/Mac
#ifdef Q_OS_WIN
        path = "C:/";
#else
        path = "/";
#endif
    }

    QDir dir(path);
    if (!dir.exists())
    {
        return "ERROR: Directory does not exist: " + path;
    }

    QFileInfoList entries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name
    );

    QStringList result;
    result.append("PATH:" + dir.absolutePath());

    for (const QFileInfo& info : entries)
    {
        QString type = info.isDir() ? "DIR" : "FILE";
        QString name = info.fileName();
        QString size = info.isDir() ? "" : QString::number(info.size());
        QString lastModified = info.lastModified().toString("yyyy-MM-dd HH:mm:ss");

        // Format: TYPE|NAME|SIZE|LAST_MODIFIED
        result.append(type + "|" + name + "|" + size + "|" + lastModified);
    }

    return result.join("\n");
}


//---------------------------------------------------
// Đọc file và trả về base64
//---------------------------------------------------

QString FileModule::downloadFile(const QString& filePath)
{
    QFile file(filePath);

    if (!file.exists())
    {
        return "ERROR: File does not exist";
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        return "ERROR: Cannot open file";
    }

    QByteArray fileData = file.readAll();
    file.close();

    // Trả về tên file và nội dung base64
    QFileInfo info(filePath);
    QString fileName = info.fileName();

    // Format: FILENAME:base64data
    QString result = fileName + ":" + fileData.toBase64();

    qDebug() << "File read:" << filePath
             << "Size:" << fileData.size() << "bytes";

    return result;
}
