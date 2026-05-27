#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    loadUsers();
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

bool LoginWidget::loadUsers()
{
    QFile file("users.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        users = doc.object();
        file.close();
        return true;
    }
    return false;
}

bool LoginWidget::saveUsers()
{
    QFile file("users.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(users);
        file.write(doc.toJson());
        file.close();
        return true;
    }
    return false;
}

bool LoginWidget::checkUser(const QString &username, const QString &password)
{
    if (users.contains(username)) {
        return users[username].toString() == password;
    }
    return false;
}

bool LoginWidget::registerUser(const QString &username, const QString &password)
{
    if (users.contains(username)) {
        return false;
    }
    users[username] = password;
    return saveUsers();
}

void LoginWidget::on_loginBtn_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }

    if (checkUser(username, password)) {
        QMessageBox::information(this, "成功", "登录成功！");
        emit loginSuccess(username);
    } else {
        QMessageBox::warning(this, "失败", "用户名或密码错误");
    }
}

void LoginWidget::on_registerBtn_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }

    if (registerUser(username, password)) {
        QMessageBox::information(this, "成功", "注册成功！请登录");
        ui->passwordEdit->clear();
    } else {
        QMessageBox::warning(this, "失败", "用户名已存在");
    }
}

void LoginWidget::on_clearBtn_clicked()
{
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
}