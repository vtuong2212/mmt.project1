#include "ProcessModule.h"

#include <QProcess>
#include <QDebug>

ProcessModule::ProcessModule(QObject *parent)
    : QObject(parent)
{
}


//---------------------------------------------------
// Liệt kê tất cả tiến trình đang chạy
// Dùng tasklist /FO CSV trên Windows
//---------------------------------------------------

QString ProcessModule::listProcesses()
{
#ifdef Q_OS_WIN
    QProcess process;
    process.start("tasklist", QStringList() << "/FO" << "CSV" << "/NH");
    process.waitForFinished(10000);

    QString output = process.readAllStandardOutput();

    // Parse CSV format: "Image Name","PID","Session Name","Session#","Mem Usage"
    QStringList lines = output.split("\r\n", Qt::SkipEmptyParts);
    QStringList result;

    for (const QString& line : lines)
    {
        // Tách các trường trong CSV
        QStringList fields;
        QString field;
        bool inQuotes = false;

        for (int i = 0; i < line.length(); i++)
        {
            QChar c = line[i];

            if (c == '"')
            {
                inQuotes = !inQuotes;
            }
            else if (c == ',' && !inQuotes)
            {
                fields.append(field.trimmed());
                field.clear();
            }
            else
            {
                field += c;
            }
        }
        fields.append(field.trimmed());

        if (fields.size() >= 5)
        {
            // Format: Name | PID | Memory
            QString entry = fields[0] + "|" +
                            fields[1] + "|" +
                            fields[4];
            result.append(entry);
        }
    }

    return result.join("\n");
#else
    QProcess process;
    process.start("ps", QStringList() << "aux");
    process.waitForFinished(10000);
    return process.readAllStandardOutput();
#endif
}


//---------------------------------------------------
// Kill tiến trình theo PID
//---------------------------------------------------

QString ProcessModule::killProcess(const QString& pid)
{
#ifdef Q_OS_WIN
    QProcess process;
    process.start("taskkill", QStringList() << "/PID" << pid << "/F");
    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Killed process PID:" << pid;
        return "SUCCESS";
    }

    QString error = process.readAllStandardError();
    qDebug() << "Failed to kill PID:" << pid << error;
    return "FAIL: " + error.trimmed();
#else
    QProcess process;
    process.start("kill", QStringList() << "-9" << pid);
    process.waitForFinished(5000);

    return (process.exitCode() == 0) ? "SUCCESS" : "FAIL";
#endif
}


//---------------------------------------------------
// Khởi động tiến trình mới
//---------------------------------------------------

QString ProcessModule::startProcess(const QString& processPath)
{
    bool success = QProcess::startDetached(processPath, QStringList());

    if (success)
    {
        qDebug() << "Started process:" << processPath;
        return "SUCCESS";
    }

    qDebug() << "Failed to start process:" << processPath;
    return "FAIL";
}
