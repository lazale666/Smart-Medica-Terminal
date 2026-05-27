#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <QMessageBox>
#include <QColorDialog>
#include <QDir>
#include <QSettings>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    m_settings = new QSettings("SmartMedica", "Client", this);
    m_currentColor = "#000000";

    connect(ui->navUserInfoBtn, &QPushButton::clicked, this, &SettingsWidget::onNavUserInfoClicked);
    connect(ui->navModeBtn, &QPushButton::clicked, this, &SettingsWidget::onNavModeClicked);
    connect(ui->navCacheBtn, &QPushButton::clicked, this, &SettingsWidget::onNavCacheClicked);
    connect(ui->navServerBtn, &QPushButton::clicked, this, &SettingsWidget::onNavServerClicked);
    connect(ui->closeBtn, &QPushButton::clicked, this, &SettingsWidget::onCloseBtnClicked);

    connect(ui->logoutBtn, &QPushButton::clicked, this, &SettingsWidget::onLogoutBtnClicked);
    connect(ui->modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWidget::onModeChanged);
    connect(ui->colorBtn, &QPushButton::clicked, this, &SettingsWidget::onColorBtnClicked);
    connect(ui->clearCacheBtn, &QPushButton::clicked, this, &SettingsWidget::onClearCacheBtnClicked);
    connect(ui->saveServerBtn, &QPushButton::clicked, this, &SettingsWidget::onSaveServerConfigBtnClicked);

    ui->navUserInfoBtn->setChecked(true);
    switchToPage(0);

    loadSettings();
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::setUsername(const QString &username)
{
    m_currentUsername = username;
    ui->usernameLabel->setText(username);
}

void SettingsWidget::setServerConfig(const QString &ip, quint16 port, bool autoConnect)
{
    ui->ipEdit->setText(ip);
    ui->portEdit->setText(QString::number(port));
    ui->autoConnectCheck->setChecked(autoConnect);
}

void SettingsWidget::setCurrentMode(const QString &mode)
{
    if (mode == "极简模式") {
        ui->modeCombo->setCurrentIndex(1);
        ui->colorWidget->setVisible(false);
    } else if (mode == "关怀模式") {
        ui->modeCombo->setCurrentIndex(2);
        ui->colorWidget->setVisible(false);
    } else {
        ui->modeCombo->setCurrentIndex(0);
        ui->colorWidget->setVisible(true);
    }
}

void SettingsWidget::setFontColor(const QString &color)
{
    m_currentColor = color;
    ui->colorPreview->setStyleSheet(QString("background-color: %1; color: %2;")
                                    .arg(color)
                                    .arg(color == "#000000" || color == "#111111" || color == "#222222" ? "white" : "black"));
}

void SettingsWidget::switchToPage(int pageIndex)
{
    ui->pageStack->setCurrentIndex(pageIndex);

    ui->navUserInfoBtn->setChecked(pageIndex == 0);
    ui->navModeBtn->setChecked(pageIndex == 1);
    ui->navCacheBtn->setChecked(pageIndex == 2);
    ui->navServerBtn->setChecked(pageIndex == 3);
}

void SettingsWidget::loadSettings()
{
    QString mode = m_settings->value("mode", "普通模式").toString();
    QString color = m_settings->value("fontColor", "#000000").toString();
    QString ip = m_settings->value("serverIP", "127.0.0.1").toString();
    quint16 port = m_settings->value("serverPort", 9999).toUInt();
    bool autoConnect = m_settings->value("autoConnect", true).toBool();

    setCurrentMode(mode);
    setFontColor(color);
    setServerConfig(ip, port, autoConnect);
}

void SettingsWidget::saveSettings()
{
    m_settings->setValue("mode", ui->modeCombo->currentText());
    m_settings->setValue("fontColor", m_currentColor);
    m_settings->setValue("serverIP", ui->ipEdit->text());
    m_settings->setValue("serverPort", ui->portEdit->text().toUInt());
    m_settings->setValue("autoConnect", ui->autoConnectCheck->isChecked());
    m_settings->sync();
}

void SettingsWidget::onNavUserInfoClicked()
{
    switchToPage(0);
}

void SettingsWidget::onNavModeClicked()
{
    switchToPage(1);
}

void SettingsWidget::onNavCacheClicked()
{
    switchToPage(2);
}

void SettingsWidget::onNavServerClicked()
{
    switchToPage(3);
}

void SettingsWidget::onCloseBtnClicked()
{
    saveSettings();
    emit closeSettings();
}

void SettingsWidget::onLogoutBtnClicked()
{
    int ret = QMessageBox::question(this, "确认退出", "确定要退出登录吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        saveSettings();
        emit logout();
    }
}

void SettingsWidget::onModeChanged(int index)
{
    QString mode = ui->modeCombo->itemText(index);
    if (mode == "极简模式" || mode == "关怀模式") {
        ui->colorWidget->setVisible(false);
        m_currentColor = "#000000";
        ui->colorPreview->setStyleSheet("background-color: #000000; color: white;");
        saveSettings();
        emit fontColorChanged(m_currentColor);
    } else {
        ui->colorWidget->setVisible(true);
    }

    saveSettings();
    emit modeChanged(mode);
}

void SettingsWidget::onColorBtnClicked()
{
    QColor color = QColorDialog::getColor(QColor(m_currentColor), this, "选择字体颜色");
    if (color.isValid()) {
        m_currentColor = color.name();
        ui->colorPreview->setStyleSheet(QString("background-color: %1; color: %2;")
                                        .arg(m_currentColor)
                                        .arg(m_currentColor == "#000000" || m_currentColor == "#111111" || m_currentColor == "#222222" ? "white" : "black"));

        saveSettings();
        emit fontColorChanged(m_currentColor);
    }
}

void SettingsWidget::onClearCacheBtnClicked()
{
    int ret = QMessageBox::warning(this, "确认删除", "确定要删除所有历史聊天记录吗？此操作不可恢复！",
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString historyDir = appDir + "/chat_history";
        QDir dir(historyDir);

        if (dir.exists()) {
            QStringList filters;
            filters << "chat_*.txt";
            QStringList files = dir.entryList(filters, QDir::Files);

            int deletedCount = 0;
            for (const QString &file : files) {
                if (dir.remove(file)) {
                    deletedCount++;
                }
            }

            QMessageBox::information(this, "删除成功", QString("成功删除 %1 条历史记录").arg(deletedCount));
            emit cacheCleared();
        } else {
            QMessageBox::information(this, "提示", "没有找到历史记录目录");
        }
    }
}

void SettingsWidget::onSaveServerConfigBtnClicked()
{
    QString ip = ui->ipEdit->text().trimmed();
    QString portStr = ui->portEdit->text().trimmed();

    if (ip.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入服务器IP");
        return;
    }

    bool ok;
    quint16 port = portStr.toUInt(&ok);
    if (!ok || port == 0 || port > 65535) {
        QMessageBox::warning(this, "错误", "请输入有效的端口号（1-65535）");
        return;
    }

    saveSettings();
    emit serverConfigChanged(ip, port, ui->autoConnectCheck->isChecked());
    QMessageBox::information(this, "保存成功", "服务器配置已保存");
}