#include "settingswidget_doc.h"
#include "ui_settingswidget_doc.h"

#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>

SettingsWidget_Doc::SettingsWidget_Doc(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget_Doc)
    , m_settings(new QSettings("SmartMedica", "DoctorClient", this))
{
    ui->setupUi(this);
    setMinimumSize(620, 520);
    setWindowTitle(QStringLiteral("设置"));

    ui->navUserInfoBtn->setText(QStringLiteral("用户信息"));
    ui->navCacheBtn->setText(QStringLiteral("缓存管理"));
    ui->navServerBtn->setText(QStringLiteral("服务器配置"));
    ui->userIconLabel->setText(QStringLiteral("医师"));
    ui->usernameLabel->setText(QStringLiteral("用户名"));
    ui->genderLabel->setText(QStringLiteral("性别："));
    ui->ageLabel->setText(QStringLiteral("年龄："));
    ui->genderCombo->setItemText(0, QStringLiteral("男"));
    ui->genderCombo->setItemText(1, QStringLiteral("女"));
    ui->genderCombo->setItemText(2, QStringLiteral("保密"));
    ui->saveUserInfoBtn->setText(QStringLiteral("保存信息"));
    ui->logoutBtn->setText(QStringLiteral("退出登录"));
    ui->cacheLabel->setText(QStringLiteral("缓存管理"));
    ui->cacheDescLabel->setText(QStringLiteral("清除医患聊天记录缓存"));
    ui->clearChatHistoryBtn->setText(QStringLiteral("清除聊天记录"));
    ui->serverLabel->setText(QStringLiteral("服务器配置"));
    ui->ipLabel->setText(QStringLiteral("服务器 IP："));
    ui->portLabel->setText(QStringLiteral("端口号："));
    ui->saveServerBtn->setText(QStringLiteral("保存配置"));

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
        QPushButton#navUserInfoBtn, QPushButton#navCacheBtn, QPushButton#navServerBtn {
            font-weight: 700;
        }
        QLineEdit, QComboBox, QSpinBox {
            background: rgba(2, 9, 20, 0.88);
            color: #EAFBFF;
            border: 1px solid rgba(0, 229, 255, 0.38);
            border-radius: 14px;
            padding: 8px 12px;
        }
        QComboBox QAbstractItemView {
            background: #081523;
            color: #EAFBFF;
            selection-background-color: #C7F4FF;
            selection-color: #03111D;
            outline: none;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
            border: 2px solid #00E5FF;
        }
        QPushButton#clearChatHistoryBtn {
            background: rgba(255, 95, 126, 0.90);
            color: #FFFFFF;
            border: 1px solid rgba(255, 160, 176, 0.85);
        }
    )");

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
    loadSettings();
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

QString SettingsWidget_Doc::scopedKey(const QString &field) const
{
    QString safeName = m_currentUsername.trimmed();
    if (safeName.isEmpty()) {
        safeName = QStringLiteral("anonymous");
    }
    safeName.replace(QRegularExpression(QStringLiteral(R"([\\/:*?"<>|\s]+)")), QStringLiteral("_"));
    return QStringLiteral("doctor/%1/%2").arg(safeName, field);
}

void SettingsWidget_Doc::loadSettings()
{
    setServerConfig(m_settings->value("serverIP", "127.0.0.1").toString(),
                    m_settings->value("serverPort", 9999).toUInt());

    const QString gender = m_settings->value(scopedKey(QStringLiteral("gender")), QStringLiteral("保密")).toString();
    const int age = m_settings->value(scopedKey(QStringLiteral("age")), 0).toInt();

    int genderIndex = 2;
    if (gender == QStringLiteral("男")) {
        genderIndex = 0;
    } else if (gender == QStringLiteral("女")) {
        genderIndex = 1;
    }

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
    m_settings->setValue(scopedKey(QStringLiteral("gender")), ui->genderCombo->currentText());
    m_settings->setValue(scopedKey(QStringLiteral("age")), ui->ageSpinBox->value());
    m_settings->sync();
    QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("用户信息已保存。"));
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
    emit serverConfigChanged(ui->ipEdit->text(), ui->portEdit->text().toUInt());
    QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("服务器配置已保存。"));
}

void SettingsWidget_Doc::onLogoutBtnClicked()
{
    if (QMessageBox::question(nullptr, QStringLiteral("确认退出"), QStringLiteral("确定要退出登录吗？"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        saveSettings();
        emit logout();
    }
}

void SettingsWidget_Doc::onClearChatHistoryClicked()
{
    if (QMessageBox::question(this, QStringLiteral("确认清除"), QStringLiteral("确定要清除所有聊天记录吗？"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        clearChatHistory();
    }
}

void SettingsWidget_Doc::clearChatHistory()
{
    QString safeName = m_currentUsername.trimmed();
    if (safeName.isEmpty()) {
        safeName = QStringLiteral("anonymous");
    }
    safeName.replace(QRegularExpression(QStringLiteral(R"([\\/:*?"<>|\s]+)")), QStringLiteral("_"));
    const QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history_doctor/" + safeName;
    QDir dir(historyDir);
    if (dir.exists()) {
        int deletedCount = 0;
        const QStringList files = dir.entryList(QStringList() << QStringLiteral("doctor_chat_*.txt"), QDir::Files);
        for (const QString &fileName : files) {
            if (dir.remove(fileName)) {
                ++deletedCount;
            }
        }
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("已清除 %1 条聊天记录。").arg(deletedCount));
    } else {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("暂无聊天记录。"));
    }
}
