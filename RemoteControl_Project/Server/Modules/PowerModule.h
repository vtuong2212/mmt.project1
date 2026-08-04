#ifndef POWERMODULE_H
#define POWERMODULE_H

#include <QObject>
#include <QString>

class PowerModule : public QObject
{
    Q_OBJECT

public:
    explicit PowerModule(QObject *parent = nullptr);

    // Tắt máy
    QString shutdown();

    // Khởi động lại
    QString restart();

    // Chế độ ngủ
    QString sleep();

    // Đăng xuất
    QString logoff();
};

#endif // POWERMODULE_H
