#include "widget.h"
#include "loginwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWidget login;
    Widget w;

    QObject::connect(&login, &LoginWidget::loginSuccess, [&](const QString &username) {
        login.hide();
        w.setWindowTitle("医疗智能体 - " + username);
        w.show();
    });

    login.show();

    return QApplication::exec();
}