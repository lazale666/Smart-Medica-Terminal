#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include <QMessageBox>

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    void loginSuccess(const QString &username);

private slots:
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();
    void on_clearBtn_clicked();

private:
    Ui::LoginWidget *ui;
    bool checkUser(const QString &username, const QString &password);
    bool registerUser(const QString &username, const QString &password);
    bool loadUsers();
    bool saveUsers();

    QJsonObject users;
};

#endif // LOGINWIDGET_H