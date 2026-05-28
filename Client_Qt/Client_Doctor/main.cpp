#include "doctorchatwidget.h"
#include "loginwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWidget *login = new LoginWidget();
    DoctorChatWidget *doctorChat = nullptr;

    QObject::connect(login, &LoginWidget::loginSuccessWithSocket, [&](const QString &username, QTcpSocket *socket) {
        login->hide();
        if (doctorChat) {
            delete doctorChat;
        }
        doctorChat = new DoctorChatWidget(socket);
        doctorChat->setWindowTitle("医生咨询中心 - " + username);
        doctorChat->show();
    });

    login->show();

    return QApplication::exec();
}