#include "ApplicationModule.h"

#include <QProcess>
#include <QSettings>
#include <QDebug>

ApplicationModule::ApplicationModule(QObject *parent)
    : QObject(parent)
{
}


//---------------------------------------------------
// Liệt kê ứng dụng đã cài đặt
// Đọc từ Windows Registry
//---------------------------------------------------

QString ApplicationModule::listApplications()
{
    QStringList appList;

#ifdef Q_OS_WIN
    // Đọc từ Registry: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
    QStringList registryPaths = {
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };

    for (const QString& regPath : registryPaths)
    {
        QSettings registry(regPath, QSettings::NativeFormat);

        QStringList groups = registry.childGroups();
        for (const QString& group : groups)
        {
            registry.beginGroup(group);

            QString displayName = registry.value("DisplayName").toString();
            QString installLocation = registry.value("InstallLocation").toString();

            if (!displayName.isEmpty())
            {
                QString entry = displayName;
                if (!installLocation.isEmpty())
                {
                    entry += " | " + installLocation;
                }
                if (!appList.contains(entry))
                {
                    appList.append(entry);
                }
            }

            registry.endGroup();
        }
    }

    appList.sort(Qt::CaseInsensitive);
#else
    appList.append("(Application listing only supported on Windows)");
#endif

    return appList.join("\n");
}


//---------------------------------------------------
// Mở ứng dụng theo đường dẫn
//---------------------------------------------------

QString ApplicationModule::openApplication(const QString& appPath)
{
    bool success = QProcess::startDetached(appPath, QStringList());

    if (success)
    {
        qDebug() << "Opened application:" << appPath;
        return "SUCCESS";
    }

    qDebug() << "Failed to open application:" << appPath;
    return "FAIL";
}


//---------------------------------------------------
// Đóng ứng dụng theo tên
//---------------------------------------------------

QString ApplicationModule::closeApplication(const QString& appName)
{
#ifdef Q_OS_WIN
    QProcess process;
    process.start("taskkill", QStringList() << "/IM" << appName << "/F");
    process.waitForFinished(5000);

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (process.exitCode() == 0)
    {
        qDebug() << "Closed application:" << appName;
        return "SUCCESS";
    }

    qDebug() << "Failed to close:" << appName << error;
    return "FAIL: " + error.trimmed();
#else
    Q_UNUSED(appName)
    return "FAIL: Only supported on Windows";
#endif
}
