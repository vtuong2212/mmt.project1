#include "PowerModule.h"

#include <QProcess>
#include <QDebug>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <PowrProf.h>
// Link: cần thêm LIBS += -lpowrprof trong .pro file
#endif

PowerModule::PowerModule(QObject *parent)
    : QObject(parent)
{
}


//---------------------------------------------------
// Tắt máy
//---------------------------------------------------

QString PowerModule::shutdown()
{
    qDebug() << "Executing SHUTDOWN...";

#ifdef Q_OS_WIN
    // shutdown /s /t 0 : tắt ngay lập tức
    QProcess::startDetached("shutdown", QStringList() << "/s" << "/t" << "0");
    return "SUCCESS";
#elif defined(Q_OS_MACOS)
    QProcess process;
    process.start("osascript", QStringList() << "-e" << "tell application \"System Events\" to shut down");
    process.waitForFinished();
    if (process.exitCode() == 0) {
        qDebug() << "macOS shutdown command executed successfully.";
        return "SUCCESS";
    } else {
        qDebug() << "macOS shutdown failed:" << process.readAllStandardError();
        return "FAILED";
    }
#else
    QProcess process;
    process.start("shutdown", QStringList() << "-h" << "now");
    process.waitForFinished();
    if (process.exitCode() == 0) return "SUCCESS";
    return "FAILED";
#endif
}


//---------------------------------------------------
// Khởi động lại
//---------------------------------------------------

QString PowerModule::restart()
{
    qDebug() << "Executing RESTART...";

#ifdef Q_OS_WIN
    QProcess::startDetached("shutdown", QStringList() << "/r" << "/t" << "0");
    return "SUCCESS";
#elif defined(Q_OS_MACOS)
    QProcess process;
    process.start("osascript", QStringList() << "-e" << "tell application \"System Events\" to restart");
    process.waitForFinished();
    if (process.exitCode() == 0) {
        qDebug() << "macOS restart command executed successfully.";
        return "SUCCESS";
    } else {
        qDebug() << "macOS restart failed:" << process.readAllStandardError();
        return "FAILED";
    }
#else
    QProcess process;
    process.start("shutdown", QStringList() << "-r" << "now");
    process.waitForFinished();
    if (process.exitCode() == 0) return "SUCCESS";
    return "FAILED";
#endif
}


//---------------------------------------------------
// Chế độ ngủ (Sleep/Suspend)
//---------------------------------------------------

QString PowerModule::sleep()
{
    qDebug() << "Executing SLEEP...";

#ifdef Q_OS_WIN
    // Dùng PowrProf API
    SetSuspendState(FALSE, FALSE, FALSE);
    return "SUCCESS";
#elif defined(Q_OS_MACOS)
    QProcess process;
    process.start("pmset", QStringList() << "sleepnow");
    process.waitForFinished();
    if (process.exitCode() == 0) {
        qDebug() << "macOS sleep command executed successfully.";
        return "SUCCESS";
    } else {
        qDebug() << "macOS sleep failed:" << process.readAllStandardError();
        return "FAILED";
    }
#else
    QProcess process;
    process.start("systemctl", QStringList() << "suspend");
    process.waitForFinished();
    if (process.exitCode() == 0) return "SUCCESS";
    return "FAILED";
#endif
}


//---------------------------------------------------
// Đăng xuất (Log Off)
//---------------------------------------------------

QString PowerModule::logoff()
{
    qDebug() << "Executing LOG OFF...";

#ifdef Q_OS_WIN
    ExitWindowsEx(EWX_LOGOFF, 0);
    return "SUCCESS";
#elif defined(Q_OS_MACOS)
    QProcess process;
    process.start("osascript", QStringList() << "-e" << "tell application \"System Events\" to log out");
    process.waitForFinished();
    if (process.exitCode() == 0) {
        qDebug() << "macOS logoff command executed successfully.";
        return "SUCCESS";
    } else {
        qDebug() << "macOS logoff failed:" << process.readAllStandardError();
        return "FAILED";
    }
#else
    QProcess process;
    process.start("pkill", QStringList() << "-KILL" << "-u" << qgetenv("USER"));
    process.waitForFinished();
    if (process.exitCode() == 0) return "SUCCESS";
    return "FAILED";
#endif
}
