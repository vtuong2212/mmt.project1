/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *connectionLayout;
    QLabel *labelIP;
    QLineEdit *inputIP;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect;
    QLabel *labelStatus;
    QFrame *separator;
    QTabWidget *tabWidget;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1000, 700);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setObjectName("mainLayout");
        connectionLayout = new QHBoxLayout();
        connectionLayout->setObjectName("connectionLayout");
        labelIP = new QLabel(centralwidget);
        labelIP->setObjectName("labelIP");

        connectionLayout->addWidget(labelIP);

        inputIP = new QLineEdit(centralwidget);
        inputIP->setObjectName("inputIP");
        inputIP->setMinimumSize(QSize(250, 30));

        connectionLayout->addWidget(inputIP);

        btnConnect = new QPushButton(centralwidget);
        btnConnect->setObjectName("btnConnect");
        btnConnect->setMinimumSize(QSize(100, 30));

        connectionLayout->addWidget(btnConnect);

        btnDisconnect = new QPushButton(centralwidget);
        btnDisconnect->setObjectName("btnDisconnect");
        btnDisconnect->setMinimumSize(QSize(100, 30));

        connectionLayout->addWidget(btnDisconnect);

        labelStatus = new QLabel(centralwidget);
        labelStatus->setObjectName("labelStatus");

        connectionLayout->addWidget(labelStatus);


        mainLayout->addLayout(connectionLayout);

        separator = new QFrame(centralwidget);
        separator->setObjectName("separator");
        separator->setFrameShape(QFrame::Shape::HLine);
        separator->setFrameShadow(QFrame::Shadow::Sunken);

        mainLayout->addWidget(separator);

        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");

        mainLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Remote Control - Client", nullptr));
        labelIP->setText(QCoreApplication::translate("MainWindow", "Server IP:", nullptr));
        labelIP->setStyleSheet(QCoreApplication::translate("MainWindow", "font-weight: bold;", nullptr));
        inputIP->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter server IP address (e.g., 192.168.1.100)", nullptr));
        inputIP->setText(QCoreApplication::translate("MainWindow", "127.0.0.1", nullptr));
        btnConnect->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        btnConnect->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #27ae60; color: white; font-weight: bold; border-radius: 5px;", nullptr));
        btnDisconnect->setText(QCoreApplication::translate("MainWindow", "Disconnect", nullptr));
        btnDisconnect->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #e74c3c; color: white; font-weight: bold; border-radius: 5px;", nullptr));
        labelStatus->setText(QCoreApplication::translate("MainWindow", "\342\232\253 Disconnected", nullptr));
        labelStatus->setStyleSheet(QCoreApplication::translate("MainWindow", "color: #e74c3c; font-weight: bold; padding: 5px;", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
