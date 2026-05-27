#include "widget.h"
#include "loginwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWidget *login = new LoginWidget();
    Widget *w = new Widget();

    QObject::connect(login, &LoginWidget::loginSuccess, [=](const QString &username) {
        login->hide();
        w->setUsername(username);
        w->setWindowTitle("医疗智能体 - " + username);
        w->show();
    });

    QObject::connect(w, &Widget::logout, [=]() {
        w->hide();
        login->show();
    });

    login->show();

    return a.exec();
}