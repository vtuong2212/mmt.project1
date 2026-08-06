#include "ApplicationModule.h"

#include <QProcess>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSet>

ApplicationModule::ApplicationModule(QObject *parent)
    : QObject(parent)
{
}


//=========================================
// 1. LIST APPLICATIONS
//=========================================

QString ApplicationModule::listApplications()
{
    QStringList appList;

#ifdef Q_OS_WIN
    qDebug() << "Received LIST_APPLICATION (Windows)";

    // Đọc từ 3 nhánh Registry
    QStringList registryPaths = {
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };

    QSet<QString> seen; // tránh trùng

    for (const QString& regPath : registryPaths)
    {
        QSettings registry(regPath, QSettings::NativeFormat);
        QStringList groups = registry.childGroups();

        for (const QString& group : groups)
        {
            registry.beginGroup(group);

            QString displayName     = registry.value("DisplayName").toString().trimmed();
            QString installLocation = registry.value("InstallLocation").toString().trimmed();
            QString displayIcon     = registry.value("DisplayIcon").toString().trimmed();
            QString systemComponent = registry.value("SystemComponent").toString();

            registry.endGroup();

            // Bỏ qua: tên rỗng, system component ẩn
            if (displayName.isEmpty()) continue;
            if (systemComponent == "1") continue;
            if (seen.contains(displayName)) continue;

            seen.insert(displayName);

            // Nếu không có InstallLocation, thử lấy từ DisplayIcon
            if (installLocation.isEmpty() && !displayIcon.isEmpty())
            {
                // DisplayIcon thường có dạng: "C:\path\to\app.exe,0"
                QString iconPath = displayIcon.split(",").first().trimmed();
                QFileInfo fi(iconPath);
                if (fi.exists())
                {
                    installLocation = fi.absolutePath();
                }
            }

            appList.append(displayName + " | " + installLocation);
        }
    }

    appList.sort(Qt::CaseInsensitive);
    qDebug() << "Found" << appList.size() << "applications. Sending...";

#elif defined(Q_OS_MACOS)
    qDebug() << "Received LIST_APPLICATION (macOS)";

    QSet<QString> seen;

    // Quét /Applications và /System/Applications
    QStringList scanDirs = {
        "/Applications",
        "/System/Applications",
        QDir::homePath() + "/Applications"
    };

    for (const QString& dirPath : scanDirs)
    {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        QFileInfoList entries = dir.entryInfoList(
            QStringList() << "*.app",
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name
        );

        for (const QFileInfo& fi : entries)
        {
            QString appName = fi.baseName();
            QString appPath = fi.absoluteFilePath();

            if (seen.contains(appName)) continue;
            seen.insert(appName);

            appList.append(appName + " | " + appPath);
        }
    }

    appList.sort(Qt::CaseInsensitive);
    qDebug() << "Found" << appList.size() << "applications. Sending...";

#else
    appList.append("(Not supported on this OS) | ");
#endif

    return appList.join("\n");
}


//=========================================
// 2. OPEN APPLICATION
//=========================================

QString ApplicationModule::openApplication(const QString& input)
{
    QString appName = input.trimmed();

    qDebug() << "Received OPEN_APPLICATION:" << appName;

#ifdef Q_OS_WIN
    // ── Bước 1: Thử tìm trong Registry để lấy đường dẫn exe ──
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

            QString displayName     = registry.value("DisplayName").toString().trimmed();
            QString installLocation = registry.value("InstallLocation").toString().trimmed();
            QString displayIcon     = registry.value("DisplayIcon").toString().trimmed();

            registry.endGroup();

            // So sánh không phân biệt hoa thường
            if (displayName.compare(appName, Qt::CaseInsensitive) != 0) continue;

            // Có InstallLocation → tìm exe trong đó
            if (!installLocation.isEmpty())
            {
                QDir installDir(installLocation);
                QFileInfoList exeFiles = installDir.entryInfoList(
                    QStringList() << "*.exe",
                    QDir::Files
                );

                if (!exeFiles.isEmpty())
                {
                    QString exePath = exeFiles.first().absoluteFilePath();
                    bool ok = QProcess::startDetached(exePath, {});
                    if (ok)
                    {
                        qDebug() << "Opening" << appName << "via registry path:" << exePath;
                        qDebug() << "Success.";
                        return "SUCCESS";
                    }
                }
            }

            // Không có InstallLocation → thử DisplayIcon
            if (!displayIcon.isEmpty())
            {
                QString iconExe = displayIcon.split(",").first().trimmed();
                if (QFileInfo::exists(iconExe))
                {
                    bool ok = QProcess::startDetached(iconExe, {});
                    if (ok)
                    {
                        qDebug() << "Opening" << appName << "via DisplayIcon:" << iconExe;
                        qDebug() << "Success.";
                        return "SUCCESS";
                    }
                }
            }
        }
    }

    // ── Bước 2: Name mapping cho app phổ biến ──
    static const QMap<QString, QString> windowsAppMap = {
        {"chrome",      "chrome.exe"},
        {"google chrome", "chrome.exe"},
        {"firefox",     "firefox.exe"},
        {"edge",        "msedge.exe"},
        {"microsoft edge", "msedge.exe"},
        {"notepad",     "notepad.exe"},
        {"calculator",  "calc.exe"},
        {"calc",        "calc.exe"},
        {"paint",       "mspaint.exe"},
        {"mspaint",     "mspaint.exe"},
        {"word",        "winword.exe"},
        {"excel",       "excel.exe"},
        {"powerpoint",  "powerpnt.exe"},
        {"outlook",     "outlook.exe"},
        {"vscode",      "code.exe"},
        {"visual studio code", "code.exe"},
        {"code",        "code.exe"},
        {"spotify",     "spotify.exe"},
        {"discord",     "discord.exe"},
        {"zoom",        "zoom.exe"},
        {"teams",       "teams.exe"},
        {"slack",       "slack.exe"},
        {"telegram",    "telegram.exe"},
        {"zalo",        "zalo.exe"},
        {"vlc",         "vlc.exe"},
        {"explorer",    "explorer.exe"},
        {"cmd",         "cmd.exe"},
        {"powershell",  "powershell.exe"},
        {"wordpad",     "wordpad.exe"},
    };

    QString key = appName.toLower().trimmed();

    if (windowsAppMap.contains(key))
    {
        QString exe = windowsAppMap[key];
        bool ok = QProcess::startDetached(exe, {});
        if (ok)
        {
            qDebug() << "Opening" << appName << "via mapping:" << exe;
            qDebug() << "Success.";
            return "SUCCESS";
        }
    }

    // ── Bước 3: Thử chạy trực tiếp tên nhập vào ──
    bool ok = QProcess::startDetached(appName, {});
    if (ok)
    {
        qDebug() << "Opening" << appName << "directly. Success.";
        return "SUCCESS";
    }

    qDebug() << "Failed to open:" << appName;
    return "FAIL: Cannot find or open application: " + appName;

#elif defined(Q_OS_MACOS)
    // ── Bước 1: Name mapping cho macOS ──
    // Tên user nhập → tên app thực trên macOS
    static const QMap<QString, QString> macAppMap = {
        {"chrome",      "Google Chrome"},
        {"google chrome", "Google Chrome"},
        {"firefox",     "Firefox"},
        {"safari",      "Safari"},
        {"edge",        "Microsoft Edge"},
        {"microsoft edge", "Microsoft Edge"},
        {"vscode",      "Visual Studio Code"},
        {"visual studio code", "Visual Studio Code"},
        {"code",        "Visual Studio Code"},
        {"terminal",    "Terminal"},
        {"iterm",       "iTerm"},
        {"iterm2",      "iTerm"},
        {"calculator",  "Calculator"},
        {"calendar",    "Calendar"},
        {"notes",       "Notes"},
        {"mail",        "Mail"},
        {"maps",        "Maps"},
        {"music",       "Music"},
        {"photos",      "Photos"},
        {"finder",      "Finder"},
        {"preview",     "Preview"},
        {"textedit",    "TextEdit"},
        {"pages",       "Pages"},
        {"numbers",     "Numbers"},
        {"keynote",     "Keynote"},
        {"xcode",       "Xcode"},
        {"spotify",     "Spotify"},
        {"discord",     "Discord"},
        {"zoom",        "zoom.us"},
        {"slack",       "Slack"},
        {"telegram",    "Telegram"},
        {"zalo",        "Zalo"},
        {"vlc",         "VLC"},
        {"word",        "Microsoft Word"},
        {"excel",       "Microsoft Excel"},
        {"powerpoint",  "Microsoft PowerPoint"},
        {"outlook",     "Microsoft Outlook"},
        {"teams",       "Microsoft Teams"},
        {"system preferences", "System Preferences"},
        {"system settings", "System Settings"},
        {"activity monitor", "Activity Monitor"},
        {"automator",   "Automator"},
    };

    QString key = appName.toLower().trimmed();
    QString realName;

    // Kiểm tra mapping
    if (macAppMap.contains(key))
    {
        realName = macAppMap[key];
    }
    else
    {
        // Thử capitalize: "safari" → "Safari"
        realName = appName;
        if (!realName.isEmpty())
        {
            realName[0] = realName[0].toUpper();
        }
    }

    // ── Bước 2: Thử tìm file .app trực tiếp ──
    QStringList searchDirs = {
        "/Applications/" + realName + ".app",
        "/System/Applications/" + realName + ".app",
        QDir::homePath() + "/Applications/" + realName + ".app",
        "/Applications/" + appName + ".app",
        "/System/Applications/" + appName + ".app",
    };

    for (const QString& path : searchDirs)
    {
        if (QDir(path).exists())
        {
            QProcess process;
            process.start("open", QStringList() << path);
            process.waitForFinished(5000);

            if (process.exitCode() == 0)
            {
                qDebug() << "Opening" << appName << "via path:" << path;
                qDebug() << "Success.";
                return "SUCCESS";
            }
        }
    }

    // ── Bước 3: Dùng open -a <tên> ──
    QProcess process;
    process.start("open", QStringList() << "-a" << realName);
    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Opening" << appName << "via open -a:" << realName;
        qDebug() << "Success.";
        return "SUCCESS";
    }

    // Thử lại với tên gốc user nhập
    if (realName != appName)
    {
        process.start("open", QStringList() << "-a" << appName);
        process.waitForFinished(5000);

        if (process.exitCode() == 0)
        {
            qDebug() << "Opening" << appName << "via open -a (original). Success.";
            return "SUCCESS";
        }
    }

    QString err = process.readAllStandardError().trimmed();
    qDebug() << "Failed to open:" << appName << "-" << err;
    return "FAIL: Cannot find or open application: " + appName;

#else
    Q_UNUSED(input)
    return "FAIL: Not supported on this OS";
#endif
}


//=========================================
// 3. CLOSE APPLICATION
//=========================================

QString ApplicationModule::closeApplication(const QString& input)
{
    QString appName = input.trimmed();

    qDebug() << "Received CLOSE_APPLICATION:" << appName;

#ifdef Q_OS_WIN
    // Mapping tên thân thiện → tên process .exe
    static const QMap<QString, QString> processMap = {
        {"chrome",      "chrome.exe"},
        {"google chrome", "chrome.exe"},
        {"firefox",     "firefox.exe"},
        {"edge",        "msedge.exe"},
        {"microsoft edge", "msedge.exe"},
        {"notepad",     "notepad.exe"},
        {"calculator",  "calculator.exe"},
        {"calc",        "calculator.exe"},
        {"paint",       "mspaint.exe"},
        {"word",        "winword.exe"},
        {"excel",       "excel.exe"},
        {"powerpoint",  "powerpnt.exe"},
        {"outlook",     "outlook.exe"},
        {"vscode",      "code.exe"},
        {"visual studio code", "code.exe"},
        {"code",        "code.exe"},
        {"spotify",     "spotify.exe"},
        {"discord",     "discord.exe"},
        {"zoom",        "zoom.exe"},
        {"teams",       "teams.exe"},
        {"slack",       "slack.exe"},
        {"telegram",    "telegram.exe"},
        {"zalo",        "zalo.exe"},
        {"vlc",         "vlc.exe"},
        {"explorer",    "explorer.exe"},
    };

    QString key = appName.toLower().trimmed();
    QString processName = appName;

    if (processMap.contains(key))
    {
        processName = processMap[key];
    }
    else if (!processName.endsWith(".exe", Qt::CaseInsensitive))
    {
        processName += ".exe";
    }

    QProcess process;
    process.start("taskkill", QStringList() << "/IM" << processName << "/F");
    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Closing" << appName << "(" << processName << "). Done.";
        return "SUCCESS";
    }

    // Thử lại với tên gốc nếu đã thêm .exe
    QString error = process.readAllStandardError().trimmed();
    qDebug() << "Failed to close:" << appName << "-" << error;
    return "FAIL: " + error;

#elif defined(Q_OS_MACOS)
    // Mapping tên thân thiện → tên process trên macOS
    static const QMap<QString, QString> processMap = {
        {"chrome",      "Google Chrome"},
        {"google chrome", "Google Chrome"},
        {"firefox",     "Firefox"},
        {"safari",      "Safari"},
        {"edge",        "Microsoft Edge"},
        {"microsoft edge", "Microsoft Edge"},
        {"vscode",      "Electron"},      // VSCode dùng Electron
        {"visual studio code", "Electron"},
        {"code",        "Electron"},
        {"terminal",    "Terminal"},
        {"calculator",  "Calculator"},
        {"notes",       "Notes"},
        {"mail",        "Mail"},
        {"finder",      "Finder"},
        {"preview",     "Preview"},
        {"textedit",    "TextEdit"},
        {"spotify",     "Spotify"},
        {"discord",     "Discord"},
        {"zoom",        "zoom.us"},
        {"slack",       "Slack"},
        {"telegram",    "Telegram"},
        {"zalo",        "Zalo"},
        {"vlc",         "VLC"},
        {"word",        "Microsoft Word"},
        {"excel",       "Microsoft Excel"},
        {"powerpoint",  "Microsoft PowerPoint"},
        {"outlook",     "Microsoft Outlook"},
        {"teams",       "Microsoft Teams"},
    };

    QString key = appName.toLower().trimmed();
    QString processName = appName;

    if (processMap.contains(key))
    {
        processName = processMap[key];
    }
    else
    {
        // Capitalize first letter
        processName = appName;
        if (!processName.isEmpty())
            processName[0] = processName[0].toUpper();
    }

    qDebug() << "Closing" << appName << "→ process:" << processName;

    // Thử pkill -x (exact match)
    QProcess process;
    process.start("pkill", QStringList() << "-x" << processName);
    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Done.";
        return "SUCCESS";
    }

    // Thử killall
    process.start("killall", QStringList() << processName);
    process.waitForFinished(5000);

    if (process.exitCode() == 0)
    {
        qDebug() << "Done (killall).";
        return "SUCCESS";
    }

    // Thử với tên gốc user nhập
    if (processName != appName)
    {
        process.start("pkill", QStringList() << "-x" << appName);
        process.waitForFinished(5000);

        if (process.exitCode() == 0)
        {
            qDebug() << "Done.";
            return "SUCCESS";
        }
    }

    QString error = process.readAllStandardError().trimmed();
    qDebug() << "Failed to close:" << appName;
    return "FAIL: Cannot close application: " + appName;

#else
    Q_UNUSED(input)
    return "FAIL: Not supported on this OS";
#endif
}
