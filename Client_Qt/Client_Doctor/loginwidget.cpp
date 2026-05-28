#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "../Client/resourcepaths.h"

#include <QBrush>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPalette>
#include <QPixmap>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_bgPath(ResourcePaths::findPhoto("background.png"))
{
    ui->setupUi(this);

    setFixedSize(1017, 398);
    ui->usernameEdit->setMaximumWidth(420);
    ui->passwordEdit->setMaximumWidth(420);
    ui->loginBtn->setFixedWidth(170);
    ui->registerBtn->setFixedWidth(170);
    ui->verticalLayout->setAlignment(ui->usernameEdit, Qt::AlignHCenter);
    ui->verticalLayout->setAlignment(ui->passwordEdit, Qt::AlignHCenter);
    ui->verticalLayout->setAlignment(ui->horizontalLayout, Qt::AlignHCenter);
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
        QPushButton:disabled {
            background: rgba(92, 112, 130, 0.75);
            color: rgba(234, 251, 255, 0.55);
        }
    )");
    ui->titleLabel->setStyleSheet("color: #EAFBFF; font: 700 30px \"Microsoft YaHei\";");
    updateBackground();
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_loginBtn_clicked()
{
    m_username = ui->usernameEdit->text().trimmed();
    m_password = ui->passwordEdit->text();

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请输入用户名和密码"));
        return;
    }

    if (!verifyLocalCredentials(m_username, m_password)) {
        QMessageBox::warning(nullptr, QStringLiteral("登录失败"), QStringLiteral("用户名或密码错误"));
        return;
    }

    emit loginSuccess(m_username);
    emit localLoginSuccess(m_username, m_password);
}

void LoginWidget::on_registerBtn_clicked()
{
    m_username = ui->usernameEdit->text().trimmed();
    m_password = ui->passwordEdit->text();

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请输入用户名和密码"));
        return;
    }

    if (!registerLocalUser(m_username, m_password)) {
        QMessageBox::warning(nullptr, QStringLiteral("注册失败"), QStringLiteral("用户名已存在"));
        return;
    }

    QMessageBox::information(nullptr, QStringLiteral("成功"), QStringLiteral("注册成功，请登录"));
    ui->passwordEdit->clear();
}

bool LoginWidget::verifyLocalCredentials(const QString &username, const QString &password)
{
    QFile file("doctors.json");
    if (!file.exists()) {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject obj = doc.object();
    if (!obj.contains(username)) {
        return false;
    }

    return obj.value(username).toString() == password;
}

bool LoginWidget::registerLocalUser(const QString &username, const QString &password)
{
    QJsonObject obj;
    QFile file("doctors.json");

    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        const QByteArray data = file.readAll();
        file.close();

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            obj = doc.object();
        }
    }

    if (obj.contains(username)) {
        return false;
    }

    obj[username] = password;

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson());
    file.close();

    return true;
}

void LoginWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBackground();
}

void LoginWidget::updateBackground()
{
    QPalette palette;
    QPixmap background(m_bgPath);
    if (!m_bgPath.isEmpty() && !background.isNull()) {
        palette.setBrush(QPalette::Window, QBrush(background.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    } else {
        palette.setBrush(QPalette::Window, QBrush(QColor("#07111F")));
    }
    setPalette(palette);
    setAutoFillBackground(true);
}
