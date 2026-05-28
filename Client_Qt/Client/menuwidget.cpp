#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "settingswidget.h"
#include "themehelpers.h"

#include <QMessageBox>
#include <QShowEvent>

MenuWidget::MenuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWidget),
    m_serverIP("127.0.0.1"),
    m_serverPort(9999),
    m_autoConnect(true),
    m_fontColor("#D8F7FF"),
    m_bgColor("#07111F"),
    m_currentMode("普通模式")
{
    ui->setupUi(this);

    m_settings = new QSettings("SmartMedica", "Client", this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &MenuWidget::onSocketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MenuWidget::onSocketDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &MenuWidget::onSocketError);

    connect(ui->chatBtn, &QPushButton::clicked, this, &MenuWidget::onChatBtnClicked);
    connect(ui->medicalRecordBtn, &QPushButton::clicked, this, &MenuWidget::onMedicalRecordBtnClicked);
    connect(ui->doctorChatBtn, &QPushButton::clicked, this, &MenuWidget::onDoctorChatBtnClicked);
    connect(ui->memberRechargeBtn, &QPushButton::clicked, this, &MenuWidget::onMemberRechargeBtnClicked);
    connect(ui->settingsBtn, &QPushButton::clicked, this, &MenuWidget::onSettingsBtnClicked);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &MenuWidget::onLogoutBtnClicked);

    loadSettings();
    updateConnectionStatus();
}

MenuWidget::~MenuWidget()
{
    delete ui;
}

void MenuWidget::setUsername(const QString &username)
{
    m_username = username;
    refreshMemberAccessState();
    ui->userLabel->setText(QStringLiteral("当前用户：") + username);
}

QString MenuWidget::getUsername() const
{
    return m_username;
}

void MenuWidget::setServerInfo(const QString &ip, int port)
{
    m_serverIP = ip;
    m_serverPort = port;
}

void MenuWidget::disconnectFromServer()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

void MenuWidget::ensureServerConnected()
{
    loadSettings();
    refreshMemberAccessState();
    if (m_autoConnect) {
        connectToServer();
    } else {
        updateConnectionStatus();
    }
}

void MenuWidget::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    applyModeSettings(mode);
    applyBgColor(bgColor);
    applyFontColor(fontColor);
}

void MenuWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    QFont titleFont = ui->titleLabel->font();
    QFont infoFont = ui->userLabel->font();
    QFont statusFont = ui->statusLabel->font();
    QFont buttonFont = ui->chatBtn->font();
    QFont smallButtonFont = ui->settingsBtn->font();

    if (mode == QStringLiteral("关怀模式")) {
        titleFont.setPointSize(34);
        infoFont.setPointSize(16);
        statusFont.setPointSize(15);
        buttonFont.setPointSize(24);
        smallButtonFont.setPointSize(15);

        ui->chatBtn->setMinimumHeight(132);
        ui->medicalRecordBtn->setMinimumHeight(132);
        ui->doctorChatBtn->setMinimumHeight(132);
        ui->memberRechargeBtn->setMinimumHeight(132);
        ui->settingsBtn->setMinimumHeight(48);
        ui->logoutBtn->setMinimumHeight(48);
        resize(980, 760);
    } else {
        titleFont.setPointSize(28);
        infoFont.setPointSize(14);
        statusFont.setPointSize(12);
        buttonFont.setPointSize(20);
        smallButtonFont.setPointSize(12);

        ui->chatBtn->setMinimumHeight(100);
        ui->medicalRecordBtn->setMinimumHeight(100);
        ui->doctorChatBtn->setMinimumHeight(100);
        ui->memberRechargeBtn->setMinimumHeight(100);
        ui->settingsBtn->setMinimumHeight(35);
        ui->logoutBtn->setMinimumHeight(35);
        resize(800, 600);
    }

    ui->titleLabel->setFont(titleFont);
    ui->userLabel->setFont(infoFont);
    ui->statusLabel->setFont(statusFont);
    ui->chatBtn->setFont(buttonFont);
    ui->medicalRecordBtn->setFont(buttonFont);
    ui->doctorChatBtn->setFont(buttonFont);
    ui->memberRechargeBtn->setFont(buttonFont);
    ui->settingsBtn->setFont(smallButtonFont);
    ui->logoutBtn->setFont(smallButtonFont);
}

void MenuWidget::applyFontColor(const QString &color)
{
    m_fontColor = color.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : color;
    ui->userLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: 700; }").arg(m_fontColor));
}

void MenuWidget::applyBgColor(const QString &color)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(color);
    if (ThemeHelpers::isLightTheme(m_bgColor)) {
        setStyleSheet(R"(
            QWidget#MenuWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF);
            }
            QLabel#titleLabel {
                color: #0F2740;
                font: 700 30px "Microsoft YaHei";
            }
            QLabel#statusLabel, QLabel#userLabel {
                color: #0F2740;
            }
            QPushButton#chatBtn, QPushButton#medicalRecordBtn, QPushButton#doctorChatBtn, QPushButton#memberRechargeBtn {
                background: rgba(255, 255, 255, 0.9);
                border: 1px solid rgba(15, 39, 64, 0.18);
                border-radius: 22px;
                color: #0F2740;
                font: 700 20px "Microsoft YaHei";
                min-height: 94px;
            }
            QPushButton#chatBtn:hover, QPushButton#medicalRecordBtn:hover, QPushButton#doctorChatBtn:hover, QPushButton#memberRechargeBtn:hover {
                background: rgba(199, 244, 255, 0.95);
            }
        )");
    } else {
        setStyleSheet(R"(
            QWidget#MenuWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023);
            }
            QLabel#titleLabel {
                color: #00E5FF;
                font: 700 30px "Microsoft YaHei";
            }
            QLabel#statusLabel {
                color: #31FFB7;
            }
            QPushButton#chatBtn, QPushButton#medicalRecordBtn, QPushButton#doctorChatBtn, QPushButton#memberRechargeBtn {
                background: rgba(2, 9, 20, 0.72);
                border: 1px solid rgba(0, 229, 255, 0.48);
                border-radius: 22px;
                color: #EAFBFF;
                font: 700 20px "Microsoft YaHei";
                min-height: 94px;
            }
            QPushButton#chatBtn:hover, QPushButton#medicalRecordBtn:hover, QPushButton#doctorChatBtn:hover, QPushButton#memberRechargeBtn:hover {
                background: rgba(0, 229, 255, 0.16);
                border-color: #00E5FF;
            }
        )");
    }
    updateConnectionStatus();
}

void MenuWidget::onChatBtnClicked()
{
    emit openChat(m_serverIP, m_serverPort, m_autoConnect);
}

void MenuWidget::onMedicalRecordBtnClicked()
{
    if (!isCurrentUserMember()) {
        showMemberRequiredMessage();
        return;
    }
    emit openMedicalRecord(m_serverIP, m_serverPort, m_autoConnect);
}

void MenuWidget::onSettingsBtnClicked()
{
    SettingsWidget *settings = new SettingsWidget();
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->setUsername(m_username);
    settings->setServerConfig(m_serverIP, m_serverPort, m_autoConnect);
    settings->setCurrentMode(m_currentMode);
    settings->setBgColor(m_bgColor);
    settings->setFontColor(m_fontColor);

    connect(settings, &SettingsWidget::logout, [=]() {
        settings->close();
        disconnectFromServer();
        emit logout();
    });

    connect(settings, &SettingsWidget::serverConfigChanged, [=](const QString &ip, quint16 port, bool autoConnect) {
        m_serverIP = ip;
        m_serverPort = port;
        m_autoConnect = autoConnect;
        if (socket->state() == QTcpSocket::ConnectedState || socket->state() == QTcpSocket::ConnectingState) {
            socket->abort();
        }
        connectToServer();
    });

    connect(settings, &SettingsWidget::fontColorChanged, [=](const QString &color) {
        applyFontColor(color);
        emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
    });

    connect(settings, &SettingsWidget::bgColorChanged, [=](const QString &color) {
        applyBgColor(color);
        emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
    });

    connect(settings, &SettingsWidget::modeChanged, [=](const QString &mode) {
        applyModeSettings(mode);
        emit modeChanged(mode);
        emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
    });

    settings->show();
    settings->raise();
    settings->activateWindow();
}

void MenuWidget::onLogoutBtnClicked()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    emit logout();
}

void MenuWidget::onSocketConnected()
{
    updateConnectionStatus();
}

void MenuWidget::onSocketDisconnected()
{
    updateConnectionStatus();
}

void MenuWidget::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    updateConnectionStatus();
}

void MenuWidget::loadSettings()
{
    m_serverIP = m_settings->value("serverIP", "127.0.0.1").toString();
    m_serverPort = m_settings->value("serverPort", 9999).toInt();
    m_autoConnect = m_settings->value("autoConnect", true).toBool();
    m_bgColor = ThemeHelpers::normalizeBgColor(m_settings->value("bgColor", "#07111F").toString());
    m_fontColor = m_settings->value("fontColor", ThemeHelpers::defaultFontColorForBg(m_bgColor)).toString();
    m_currentMode = m_settings->value("mode", "普通模式").toString();
    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
    refreshMemberAccessState();
}

void MenuWidget::saveSettings()
{
    m_settings->setValue("serverIP", m_serverIP);
    m_settings->setValue("serverPort", m_serverPort);
    m_settings->setValue("autoConnect", m_autoConnect);
}

void MenuWidget::updateConnectionStatus()
{
    const QString ok = ThemeHelpers::statusOkColor(m_bgColor);
    const QString warn = ThemeHelpers::statusWarnColor(m_bgColor);
    const QString err = ThemeHelpers::statusErrorColor(m_bgColor);

    if (socket->state() == QTcpSocket::ConnectedState) {
        ui->statusLabel->setText(QStringLiteral("服务器状态：已连接"));
        ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(ok));
    } else if (socket->state() == QTcpSocket::ConnectingState) {
        ui->statusLabel->setText(QStringLiteral("服务器状态：连接中..."));
        ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(warn));
    } else {
        ui->statusLabel->setText(QStringLiteral("服务器状态：未连接"));
        ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(err));
    }
}

void MenuWidget::connectToServer()
{
    if (socket->state() != QTcpSocket::ConnectedState && socket->state() != QTcpSocket::ConnectingState) {
        socket->connectToHost(m_serverIP, m_serverPort);
    }
}

void MenuWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadSettings();
    refreshMemberAccessState();
    if (m_autoConnect) {
        connectToServer();
    }
}

void MenuWidget::onDoctorChatBtnClicked()
{
    if (!isCurrentUserMember()) {
        showMemberRequiredMessage();
        return;
    }
    emit openDoctorChat(m_serverIP, m_serverPort);
}

void MenuWidget::onMemberRechargeBtnClicked()
{
    emit openMemberRecharge();
}

bool MenuWidget::isCurrentUserMember() const
{
    if (m_username.trimmed().isEmpty()) {
        return false;
    }

    const QString key = QString("member_%1").arg(m_username);
    return m_settings->value(key, false).toBool();
}

void MenuWidget::refreshMemberAccessState()
{
    const bool isMember = isCurrentUserMember();
    ui->medicalRecordBtn->setEnabled(isMember);
    ui->doctorChatBtn->setEnabled(isMember);
    ui->medicalRecordBtn->setToolTip(isMember ? QString() : QStringLiteral("开通会员后可使用病历记录"));
    ui->doctorChatBtn->setToolTip(isMember ? QString() : QStringLiteral("开通会员后可使用名医对话"));
}

void MenuWidget::showMemberRequiredMessage()
{
    QMessageBox::information(this, QStringLiteral("会员功能"), QStringLiteral("请先开通会员后再使用该功能。"));
}
