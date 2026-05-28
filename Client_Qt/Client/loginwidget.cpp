#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "themehelpers.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QPalette>
#include <QPixmap>
#include <QResizeEvent>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_bgPath("")
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
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
    applyAppearance(m_bgColor, m_fontColor);
    updateBackground();
}

void LoginWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBackground();
}

void LoginWidget::applyAppearance(const QString &bgColor, const QString &fontColor)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(bgColor);
    m_fontColor = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : fontColor;
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);

    setStyleSheet(QString(R"(
        QLabel#titleLabel {
            color: %1;
            font: 700 30px "Microsoft YaHei";
            letter-spacing: 1px;
        }
        QLineEdit {
            background: %2;
            border: 1px solid %3;
            border-radius: 16px;
            color: %4;
            padding: 12px 18px;
            font: 14px "Microsoft YaHei";
            min-height: 24px;
        }
        QLineEdit:focus {
            border: 2px solid #00E5FF;
            background: %5;
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
    )")
                      .arg(light ? "#0F2740" : "#EAFBFF",
                           light ? "rgba(255,255,255,0.88)" : "rgba(4, 15, 31, 0.78)",
                           light ? "rgba(15,39,64,0.20)" : "rgba(0, 229, 255, 0.75)",
                           light ? "#0F2740" : "#EAFBFF",
                           light ? "rgba(255,255,255,0.96)" : "rgba(6, 24, 45, 0.88)"));
}

void LoginWidget::updateBackground()
{
    if (m_bgPath.isEmpty()) {
        return;
    }

    QPixmap background(m_bgPath);
    if (!background.isNull()) {
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(background.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        setPalette(palette);
        setAutoFillBackground(true);
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
        users = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
        return true;
    }
    return false;
}

bool LoginWidget::saveUsers()
{
    QFile file("users.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(users).toJson());
        file.close();
        return true;
    }
    return false;
}

bool LoginWidget::checkUser(const QString &username, const QString &password)
{
    return users.contains(username) && users[username].toString() == password;
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
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请输入用户名和密码。"));
        return;
    }

    if (checkUser(username, password)) {
        QMessageBox::information(nullptr, QStringLiteral("成功"), QStringLiteral("登录成功。"));
        emit loginSuccess(username);
    } else {
        QMessageBox::warning(nullptr, QStringLiteral("失败"), QStringLiteral("用户名或密码错误。"));
    }
}

void LoginWidget::on_registerBtn_clicked()
{
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请输入用户名和密码。"));
        return;
    }

    if (registerUser(username, password)) {
        QMessageBox::information(nullptr, QStringLiteral("成功"), QStringLiteral("注册成功，请登录。"));
        ui->passwordEdit->clear();
    } else {
        QMessageBox::warning(nullptr, QStringLiteral("失败"), QStringLiteral("用户名已存在。"));
    }
}
