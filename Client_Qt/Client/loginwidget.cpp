#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "resourcepaths.h"
#include "themehelpers.h"

#include <QDialog>
#include <QFile>
#include <QJsonDocument>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_bgPath(QString())
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
{
    ui->setupUi(this);
    loadUsers();

    m_bgPath = ResourcePaths::findPhoto("background.png");

    setFixedSize(1017, 398);
    ui->usernameEdit->setMaximumWidth(420);
    ui->passwordEdit->setMaximumWidth(420);
    ui->loginBtn->setFixedWidth(170);
    ui->registerBtn->setFixedWidth(170);
    ui->verticalLayout->setAlignment(ui->usernameEdit, Qt::AlignHCenter);
    ui->verticalLayout->setAlignment(ui->passwordEdit, Qt::AlignHCenter);
    ui->verticalLayout->setAlignment(ui->horizontalLayout, Qt::AlignHCenter);
    applyAppearance(m_bgColor, m_fontColor);
    updateBackground();
}

LoginWidget::~LoginWidget()
{
    delete ui;
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

bool LoginWidget::loadUsers()
{
    QFile file("users.json");
    if (file.open(QIODevice::ReadOnly)) {
        users = QJsonDocument::fromJson(file.readAll()).object();
        return true;
    }
    return false;
}

bool LoginWidget::saveUsers()
{
    QFile file("users.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(users).toJson());
        return true;
    }
    return false;
}

bool LoginWidget::checkUser(const QString &username, const QString &password)
{
    return users.contains(username) && users.value(username).toString() == password;
}

bool LoginWidget::registerUser(const QString &username, const QString &password)
{
    if (users.contains(username)) {
        return false;
    }
    users[username] = password;
    return saveUsers();
}

void LoginWidget::showDisclaimerDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("免责声明"));
    dialog.setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    QLabel *imageLabel = new QLabel(&dialog);
    imageLabel->setAlignment(Qt::AlignCenter);
    const QString imagePath = ResourcePaths::findPhoto("dontganmao.png");
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap.scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QLabel *textLabel = new QLabel(QStringLiteral("本程序仅供娱乐，不作为医学参考价值。"), &dialog);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    QLabel *footerLabel = new QLabel(QStringLiteral("另外，别感冒"), &dialog);
    footerLabel->setAlignment(Qt::AlignCenter);

    QPushButton *okBtn = new QPushButton(QStringLiteral("确定"), &dialog);
    okBtn->setFixedWidth(120);
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    layout->addWidget(imageLabel);
    layout->addWidget(textLabel);
    layout->addWidget(footerLabel);
    layout->addWidget(okBtn, 0, Qt::AlignHCenter);
    dialog.exec();
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
        showDisclaimerDialog();
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
