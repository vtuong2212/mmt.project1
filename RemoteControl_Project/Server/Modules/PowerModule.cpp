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
#else
    QProcess::startDetached("shutdown", QStringList() << "-h" << "now");
    return "SUCCESS";
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
#else
    QProcess::startDetached("shutdown", QStringList() << "-r" << "now");
    return "SUCCESS";
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
#else
    QProcess::startDetached("systemctl", QStringList() << "suspend");
    return "SUCCESS";
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
#else
    QProcess::startDetached("pkill", QStringList() << "-KILL" << "-u" << qgetenv("USER"));
    return "SUCCESS";
#endif
}
