#include "settingswidget_doc.h"
#include "ui_settingswidget_doc.h"
#include <QMessageBox>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QTextStream>

SettingsWidget_Doc::SettingsWidget_Doc(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget_Doc)
{
    ui->setupUi(this);
    setMinimumSize(620, 520);
    setStyleSheet(R"(
        QWidget#SettingsWidget_Doc {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023);
        }
        QStackedWidget, QWidget#userInfoPage, QWidget#cachePage, QWidget#serverPage {
            background: transparent;
        }
        QLabel {
            color: #D8F7FF;
            font-family: "Microsoft YaHei";
        }
        QLabel#usernameLabel, QLabel#cacheLabel, QLabel#serverLabel {
            color: #00E5FF;
            font-weight: 700;
        }
    )");

    m_settings = new QSettings("SmartMedica", "DoctorClient", this);

    connect(ui->navUserInfoBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onNavUserInfoClicked);
    connect(ui->navCacheBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onNavCacheClicked);
    connect(ui->navServerBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onNavServerClicked);

    connect(ui->logoutBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onLogoutBtnClicked);
    connect(ui->saveServerBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onSaveServerBtnClicked);
    connect(ui->saveUserInfoBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onSaveUserInfoBtnClicked);
    connect(ui->clearChatHistoryBtn, &QPushButton::clicked, this, &SettingsWidget_Doc::onClearChatHistoryClicked);

    ui->navUserInfoBtn->setChecked(true);
    switchToPage(0);

    loadSettings();
}

SettingsWidget_Doc::~SettingsWidget_Doc()
{
    delete ui;
}

void SettingsWidget_Doc::setUsername(const QString &username)
{
    m_currentUsername = username;
    ui->usernameLabel->setText(username);
}

void SettingsWidget_Doc::setServerConfig(const QString &ip, quint16 port)
{
    ui->ipEdit->setText(ip);
    ui->portEdit->setText(QString::number(port));
}

void SettingsWidget_Doc::switchToPage(int pageIndex)
{
    ui->pageStack->setCurrentIndex(pageIndex);

    ui->navUserInfoBtn->setChecked(pageIndex == 0);
    ui->navCacheBtn->setChecked(pageIndex == 1);
    ui->navServerBtn->setChecked(pageIndex == 2);
}

void SettingsWidget_Doc::loadSettings()
{
    QString ip = m_settings->value("serverIP", "127.0.0.1").toString();
    quint16 port = m_settings->value("serverPort", 9999).toUInt();
    
    QString gender = m_settings->value("gender", "保密").toString();
    int age = m_settings->value("age", 0).toInt();

    setServerConfig(ip, port);

    int genderIndex = 2;
    if (gender == "男") genderIndex = 0;
    else if (gender == "女") genderIndex = 1;
    ui->genderCombo->setCurrentIndex(genderIndex);
    ui->ageSpinBox->setValue(age);
}

void SettingsWidget_Doc::saveSettings()
{
    m_settings->setValue("serverIP", ui->ipEdit->text());
    m_settings->setValue("serverPort", ui->portEdit->text().toUInt());
    m_settings->sync();
}

void SettingsWidget_Doc::onSaveUserInfoBtnClicked()
{
    QString gender = ui->genderCombo->currentText();
    int age = ui->ageSpinBox->value();

    m_settings->setValue("gender", gender);
    m_settings->setValue("age", age);
    m_settings->sync();

    QMessageBox::information(nullptr, "提示", "用户信息已保存！");
}

void SettingsWidget_Doc::onNavUserInfoClicked()
{
    switchToPage(0);
}

void SettingsWidget_Doc::onNavCacheClicked()
{
    switchToPage(1);
}

void SettingsWidget_Doc::onNavServerClicked()
{
    switchToPage(2);
}

void SettingsWidget_Doc::onSaveServerBtnClicked()
{
    saveSettings();
    QString ip = ui->ipEdit->text();
    quint16 port = ui->portEdit->text().toUInt();
    emit serverConfigChanged(ip, port);
    QMessageBox::information(nullptr, "提示", "服务器配置已保存");
}

void SettingsWidget_Doc::onLogoutBtnClicked()
{
    int ret = QMessageBox::question(nullptr, "确认退出", "确定要退出登录吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        saveSettings();
        emit logout();
    }
}

void SettingsWidget_Doc::onClearChatHistoryClicked()
{
    int ret = QMessageBox::question(nullptr, "确认清除", "确定要清除所有聊天记录吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        clearChatHistory();
        QMessageBox::information(nullptr, "提示", "聊天记录已清除！");
    }
}

void SettingsWidget_Doc::clearChatHistory()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString historyDir = appDir + "/chat_history_doctor";
    QDir dir(historyDir);
    
    if (dir.exists()) {
        dir.removeRecursively();
    }
}
