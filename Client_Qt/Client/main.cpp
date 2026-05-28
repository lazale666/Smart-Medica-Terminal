#include "widget.h"
#include "loginwidget.h"
#include "menuwidget.h"
#include "medicalrecordwidget.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWidget *login = new LoginWidget();
    MenuWidget *menu = new MenuWidget();
    Widget *chat = new Widget();
    MedicalRecordWidget *medicalRecord = new MedicalRecordWidget();

    QSettings settings("SmartMedica", "Client");
    QString savedMode = settings.value("mode", "普通模式").toString();

    QObject::connect(login, &LoginWidget::loginSuccess, [=](const QString &username) {
        login->hide();
        menu->setUsername(username);
        QSettings settings("SmartMedica", "Client");
        QString serverIP = settings.value("serverIP", "127.0.0.1").toString();
        int serverPort = settings.value("serverPort", 9999).toInt();
        menu->setServerInfo(serverIP, serverPort);
        menu->setWindowTitle("医疗智能体 - " + username);
        menu->show();
    });

    QObject::connect(menu, &MenuWidget::openChat, [=](const QString &serverIP, int serverPort, bool autoConnect) {
        menu->hide();
        chat->setServerInfo(serverIP, serverPort, autoConnect);
        chat->setWindowTitle("医疗智能体 - 聊天问诊");
        chat->show();
    });

    QObject::connect(menu, &MenuWidget::openMedicalRecord, [=](const QString &serverIP, int serverPort, bool autoConnect) {
        menu->hide();
        medicalRecord->setServerInfo(serverIP, serverPort);
        medicalRecord->applyModeSettings(savedMode);
        medicalRecord->setWindowTitle("医疗智能体 - 病例记录");
        medicalRecord->show();
    });

    QObject::connect(menu, &MenuWidget::logout, [=]() {
        menu->hide();
        login->show();
    });

    QObject::connect(chat, &Widget::backToMenu, [=]() {
        chat->hide();
        menu->show();
    });

    QObject::connect(medicalRecord, &MedicalRecordWidget::backToMenu, [=]() {
        medicalRecord->hide();
        menu->show();
    });

    login->show();

    return a.exec();
}
