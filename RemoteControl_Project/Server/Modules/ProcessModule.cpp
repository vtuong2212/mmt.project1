#include "ProcessModule.h"

#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QRegularExpression>

ProcessModule::ProcessModule(QObject *parent)
    : QObject(parent)
{
}


//---------------------------------------------------
// Liệt kê tất cả tiến trình đang chạy
// Windows: tasklist /FO CSV
// macOS: ps -axo pid,comm,rss
// Output format: Name|PID|Memory (cả 2 OS)
//---------------------------------------------------

QString ProcessModule::listProcesses()
{
    qDebug() << "Received LIST_PROCESS";

#ifdef Q_OS_WIN
    QProcess process;
    process.start("tasklist", QStringList() << "/FO" << "CSV" << "/NH");
    process.waitForFinished(10000);

    QString output = process.readAllStandardOutput();

    // Parse CSV format: "Image Name","PID","Session Name","Session#","Mem Usage"
    QStringList lines = output.split("\r\n", Qt::SkipEmptyParts);
    QStringList result;

    for (const QString& line : lines)
    {
        // Tách các trường trong CSV
        QStringList fields;
        QString field;
        bool inQuotes = false;

        for (int i = 0; i < line.length(); i++)
        {
            QChar c = line[i];

            if (c == '"')
            {
                inQuotes = !inQuotes;
            }
            else if (c == ',' && !inQuotes)
            {
                fields.append(field.trimmed());
                field.clear();
            }
            else
            {
                field += c;
            }
        }
        fields.append(field.trimmed());

        if (fields.size() >= 5)
        {
            // Format: Name|PID|Memory
            QString entry = fields[0] + "|" +
                            fields[1] + "|" +
                            fields[4];
            result.append(entry);
        }
    }

    qDebug() << "Found" << result.size() << "processes. Sending...";
    return result.join("\n");

#elif defined(Q_OS_MACOS)
    QProcess process;
    // pid, comm (tên process), rss (memory KB)
    process.start("ps", QStringList() << "-axo" << "pid,comm,rss");
    process.waitForFinished(10000);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    QStringList result;

    // Bỏ dòng header (PID COMM RSS)
    for (int i = 1; i < lines.size(); i++)
    {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;

        // ps output format: "  PID COMM                     RSS"
        // Tách bằng regex-like: nhiều khoảng trắng
        QStringList parts = line.split(QRegularExpression("\\s+"),
                                       Qt::SkipEmptyParts);

        if (parts.size() >= 3)
        {
            QString pid = parts[0].trimmed();
            // comm có thể chứa khoảng trắng, rss luôn là field cuối
            QString rss = parts.last().trimmed();
            // comm = tất cả giữa pid và rss
            QString comm;
            for (int j = 1; j < parts.size() - 1; j++)
            {
                if (!comm.isEmpty()) comm += " ";
                comm += parts[j];
            }

            // Chỉ lấy tên file (bỏ path)
            if (comm.contains("/"))
            {
                comm = comm.mid(comm.lastIndexOf("/") + 1);
            }

            // Memory: chuyển từ KB sang readable
            bool ok;
            long rssKB = rss.toLong(&ok);
            QString memStr;
            if (ok && rssKB > 0)
            {
                if (rssKB >= 1024)
                    memStr = QString::number(rssKB / 1024) + " MB";
                else
                    memStr = rss + " KB";
            }
            else
            {
                memStr = rss + " KB";
            }

            // Format: Name|PID|Memory (giống Windows)
            result.append(comm + "|" + pid + "|" + memStr);
        }
    }

    qDebug() << "Found" << result.size() << "processes. Sending...";
    return result.join("\n");

#else
    return "(Not supported on this OS)";
#endif
}


//---------------------------------------------------
// Kill tiến trình theo PID hoặc tên
// Windows: taskkill /PID <pid> /F hoặc taskkill /IM <name> /F
// macOS: kill -9 <pid> hoặc pkill -x <name>
//---------------------------------------------------

QString ProcessModule::killProcess(const QString& input)
{
    QString target = input.trimmed();
    qDebug() << "Received KILL_PROCESS:" << target;

    // Kiểm tra xem input là PID (số) hay tên process
    bool isPid = false;
    target.toLong(&isPid);

#ifdef Q_OS_WIN
    QProcess process;

    if (isPid)
    {
        process.start("taskkill",
                      QStringList() << "/PID" << target << "/F");
    }
    else
    {
        // Tên process → taskkill /IM
        QString processName = target;
        if (!processName.endsWith(".exe", Qt::CaseInsensitive))
            processName += ".exe";

        process.start("taskkill",
                      QStringList() << "/IM" << processName << "/F");
    }

    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Killed process:" << target << ". Done.";
        return "SUCCESS";
    }

    QString error = process.readAllStandardError().trimmed();
    qDebug() << "Failed to kill:" << target << "-" << error;
    return "FAIL: " + error;

#elif defined(Q_OS_MACOS)
    QProcess process;

    if (isPid)
    {
        process.start("kill", QStringList() << "-9" << target);
    }
    else
    {
        // Tên process → pkill
        process.start("pkill", QStringList() << "-x" << target);
    }

    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Killed process:" << target << ". Done.";
        return "SUCCESS";
    }

    // Nếu pkill thất bại, thử killall
    if (!isPid)
    {
        process.start("killall", QStringList() << target);
        process.waitForFinished(5000);

        if (process.exitCode() == 0)
        {
            qDebug() << "Killed process (killall):" << target << ". Done.";
            return "SUCCESS";
        }
    }

    QString error = process.readAllStandardError().trimmed();
    qDebug() << "Failed to kill:" << target;
    return "FAIL: " + error;

#else
    Q_UNUSED(input)
    return "FAIL: Not supported";
#endif
}


//---------------------------------------------------
// Khởi động tiến trình mới
// User chỉ cần nhập tên: Chrome, Calculator, Terminal...
// Server tự mapping theo OS
//---------------------------------------------------

QString ProcessModule::startProcess(const QString& input)
{
    QString appName = input.trimmed();
    qDebug() << "Received START_PROCESS:" << appName;

#ifdef Q_OS_WIN
    //--- Windows name mapping ---
    static const QMap<QString, QString> winMap = {
        {"chrome",       "chrome.exe"},
        {"google chrome","chrome.exe"},
        {"firefox",      "firefox.exe"},
        {"edge",         "msedge.exe"},
        {"microsoft edge","msedge.exe"},
        {"notepad",      "notepad.exe"},
        {"calculator",   "calc.exe"},
        {"calc",         "calc.exe"},
        {"terminal",     "cmd.exe"},
        {"cmd",          "cmd.exe"},
        {"powershell",   "powershell.exe"},
        {"explorer",     "explorer.exe"},
        {"paint",        "mspaint.exe"},
        {"mspaint",      "mspaint.exe"},
        {"wordpad",      "wordpad.exe"},
        {"vscode",       "code.exe"},
        {"visual studio code","code.exe"},
        {"code",         "code.exe"},
        {"word",         "winword.exe"},
        {"excel",        "excel.exe"},
        {"powerpoint",   "powerpnt.exe"},
        {"outlook",      "outlook.exe"},
        {"spotify",      "spotify.exe"},
        {"discord",      "discord.exe"},
        {"zoom",         "zoom.exe"},
        {"teams",        "teams.exe"},
        {"slack",        "slack.exe"},
        {"telegram",     "telegram.exe"},
        {"zalo",         "zalo.exe"},
        {"vlc",          "vlc.exe"},
        {"steam",        "steam.exe"},
    };

    // Ứng dụng chỉ có trên macOS
    static const QStringList macOnly = {
        "safari", "finder", "textedit", "preview",
        "pages", "numbers", "keynote", "xcode",
        "activity monitor", "automator"
    };

    QString key = appName.toLower();

    if (macOnly.contains(key))
    {
        qDebug() << appName << "not available on Windows.";
        return "FAIL: " + appName + " is not available on Windows";
    }

    // Tìm trong mapping
    QString exe = winMap.value(key, "");

    if (!exe.isEmpty())
    {
        bool ok = QProcess::startDetached(exe, {});
        if (ok)
        {
            qDebug() << "Opening" << appName << "via mapping:" << exe << ". Success.";
            return "SUCCESS";
        }
    }

    // Fallback: thử chạy trực tiếp
    bool ok = QProcess::startDetached(appName, {});
    if (ok)
    {
        qDebug() << "Opening" << appName << "directly. Success.";
        return "SUCCESS";
    }

    qDebug() << "Failed to start:" << appName;
    return "FAIL: Cannot start " + appName;

#elif defined(Q_OS_MACOS)
    //--- macOS name mapping ---
    static const QMap<QString, QString> macMap = {
        {"chrome",       "Google Chrome"},
        {"google chrome","Google Chrome"},
        {"firefox",      "Firefox"},
        {"safari",       "Safari"},
        {"edge",         "Microsoft Edge"},
        {"microsoft edge","Microsoft Edge"},
        {"vscode",       "Visual Studio Code"},
        {"visual studio code","Visual Studio Code"},
        {"code",         "Visual Studio Code"},
        {"terminal",     "Terminal"},
        {"iterm",        "iTerm"},
        {"calculator",   "Calculator"},
        {"calc",         "Calculator"},
        {"finder",       "Finder"},
        {"textedit",     "TextEdit"},
        {"editor",       "TextEdit"},
        {"preview",      "Preview"},
        {"notes",        "Notes"},
        {"mail",         "Mail"},
        {"calendar",     "Calendar"},
        {"music",        "Music"},
        {"photos",       "Photos"},
        {"pages",        "Pages"},
        {"numbers",      "Numbers"},
        {"keynote",      "Keynote"},
        {"xcode",        "Xcode"},
        {"spotify",      "Spotify"},
        {"discord",      "Discord"},
        {"zoom",         "zoom.us"},
        {"slack",        "Slack"},
        {"telegram",     "Telegram"},
        {"zalo",         "Zalo"},
        {"vlc",          "VLC"},
        {"steam",        "Steam"},
        {"word",         "Microsoft Word"},
        {"excel",        "Microsoft Excel"},
        {"powerpoint",   "Microsoft PowerPoint"},
        {"outlook",      "Microsoft Outlook"},
        {"teams",        "Microsoft Teams"},
        {"activity monitor","Activity Monitor"},
        {"system settings","System Settings"},
    };

    // Ứng dụng chỉ có trên Windows
    static const QStringList winOnly = {
        "notepad", "mspaint", "paint", "wordpad",
        "powershell", "cmd"
    };

    QString key = appName.toLower();

    if (winOnly.contains(key))
    {
        qDebug() << appName << "not available on macOS.";
        return "FAIL: " + appName + " is not available on macOS";
    }

    // Tìm trong mapping
    QString macName = macMap.value(key, "");

    if (macName.isEmpty())
    {
        // Capitalize first letter: "discord" → "Discord"
        macName = appName;
        if (!macName.isEmpty())
            macName[0] = macName[0].toUpper();
    }

    // Dùng "open -a" để mở
    QProcess process;
    process.start("open", QStringList() << "-a" << macName);
    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Opening" << appName << "→" << macName << ". Success.";
        return "SUCCESS";
    }

    // Thử tên gốc
    if (macName != appName)
    {
        process.start("open", QStringList() << "-a" << appName);
        process.waitForFinished(5000);

        if (process.exitCode() == 0)
        {
            qDebug() << "Opening" << appName << "directly. Success.";
            return "SUCCESS";
        }
    }

    QString error = process.readAllStandardError().trimmed();
    qDebug() << "Failed to start:" << appName << "-" << error;
    return "FAIL: Cannot start " + appName;

#else
    Q_UNUSED(input)
    return "FAIL: Not supported";
#endif
}
