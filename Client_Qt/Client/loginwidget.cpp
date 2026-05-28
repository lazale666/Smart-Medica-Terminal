#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QPalette>
#include <QPixmap>
#include <QCoreApplication>
#include <QFileInfo>
#include <QResizeEvent>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_bgPath("")
{
    ui->setupUi(this);
    loadUsers();

    QString bgPath = QCoreApplication::applicationDirPath() + "/photo/background.png";
    QFileInfo fileInfo(bgPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        bgPath = "D:/All Program/agant_example/Smart-Medica-Terminal/Client_Qt/Client/photo/background.png";
    }
    m_bgPath = bgPath;

    setFixedSize(1017, 398);
    setStyleSheet(R"(
        QLabel#titleLabel {
            color: #EAFBFF;
            font: 700 30px "Microsoft YaHei";
            letter-spacing: 1px;
        }
        QLineEdit {
            background: rgba(4, 15, 31, 0.78);
            border: 1px solid rgba(0, 229, 255, 0.75);
            border-radius: 16px;
            color: #EAFBFF;
            padding: 12px 18px;
            font: 14px "Microsoft YaHei";
            min-height: 24px;
        }
        QLineEdit:focus {
            border: 2px solid #00E5FF;
            background: rgba(6, 24, 45, 0.88);
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);
            border: 1px solid rgba(234, 251, 255, 0.75);
            border-radius: 16px;
            color: #03111D;
            padding: 10px 24px;
            font: 700 14px "Microsoft YaHei";
            min-height: 28px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31FFB7, stop:1 #00E5FF);
        }
    )");
    ui->titleLabel->setStyleSheet("color: #EAFBFF; font: 700 30px \"Microsoft YaHei\";");

    updateBackground();
}

void LoginWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBackground();
}

void LoginWidget::updateBackground()
{
    if (m_bgPath.isEmpty()) return;

    QPixmap background(m_bgPath);
    if (!background.isNull()) {
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        this->setPalette(palette);
        this->setAutoFillBackground(true);
    }
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
        QMessageBox::warning(nullptr, "提示", "请输入用户名和密码");
        return;
    }

    if (checkUser(username, password)) {
        QMessageBox::information(nullptr, "成功", "登录成功！");
        emit loginSuccess(username);
    } else {
        QMessageBox::warning(nullptr, "失败", "用户名或密码错误");
    }
}

void LoginWidget::on_registerBtn_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(nullptr, "提示", "请输入用户名和密码");
        return;
    }

    if (registerUser(username, password)) {
        QMessageBox::information(nullptr, "成功", "注册成功！请登录");
        ui->passwordEdit->clear();
    } else {
        QMessageBox::warning(nullptr, "失败", "用户名已存在");
    }
}
