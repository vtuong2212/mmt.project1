#ifndef FILEMODULE_H
#define FILEMODULE_H

#include <QObject>
#include <QString>

#include "../../Common/Constants.h"

class FileModule : public QObject
{
    Q_OBJECT

public:
    explicit FileModule(QObject *parent = nullptr);

    // Liệt kê file/folder trong thư mục
    QString listFiles(const QString& directoryPath);

    // Đọc file và trả về base64
    QString downloadFile(const QString& filePath);
};

#endif // FILEMODULE_H
