#ifndef PROCESSMODULE_H
#define PROCESSMODULE_H

#include <QObject>
#include <QString>

class ProcessModule : public QObject
{
    Q_OBJECT

public:
    explicit ProcessModule(QObject *parent = nullptr);

    // Liệt kê tất cả tiến trình đang chạy
    QString listProcesses();

    // Kill tiến trình theo PID
    QString killProcess(const QString& pid);

    // Khởi động tiến trình mới
    QString startProcess(const QString& processPath);
};

#endif // PROCESSMODULE_H
