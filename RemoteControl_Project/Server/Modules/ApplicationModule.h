#ifndef APPLICATIONMODULE_H
#define APPLICATIONMODULE_H

#include <QObject>
#include <QString>
#include <QStringList>

class ApplicationModule : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationModule(QObject *parent = nullptr);

    // Liệt kê ứng dụng đã cài đặt (từ Registry)
    QString listApplications();

    // Mở ứng dụng theo đường dẫn
    QString openApplication(const QString& appPath);

    // Đóng ứng dụng theo tên tiến trình
    QString closeApplication(const QString& appName);
};

#endif // APPLICATIONMODULE_H
