
#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "settingswidget.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QShowEvent>

MenuWidget::MenuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWidget),
    m_serverIP("127.0.0.1"),
    m_serverPort(9999),
    m_autoConnect(true),
    m_fontColor("#000000"),
    m_bgColor("#ffffff")
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
    ui->userLabel->setText("当前用户：" + username);
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

void MenuWidget::applyFontColor(const QString &color)
{
    m_fontColor = color;
    ui->userLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(color));
}

void MenuWidget::applyBgColor(const QString &color)
{
    m_bgColor = color;
    this->setStyleSheet(QString("QWidget { background-color: %1; }").arg(color));
}

void MenuWidget::onChatBtnClicked()
{
    emit openChat(m_serverIP, m_serverPort, m_autoConnect);
}

void MenuWidget::onMedicalRecordBtnClicked()
{
    emit openMedicalRecord(m_serverIP, m_serverPort, m_autoConnect);
}

void MenuWidget::onSettingsBtnClicked()
{
    SettingsWidget *settings = new SettingsWidget();
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->setFontColor(m_fontColor);
    settings->setBgColor(m_bgColor);
    
    QObject::connect(settings, &SettingsWidget::logout, [=]() {
        settings->close();
        disconnectFromServer();
        emit logout();
    });
    
    QObject::connect(settings, &SettingsWidget::serverConfigChanged, [=](const QString &ip, quint16 port, bool autoConnect) {
        m_serverIP = ip;
        m_serverPort = port;
        m_autoConnect = autoConnect;
    });
    
    QObject::connect(settings, &SettingsWidget::fontColorChanged, [=](const QString &color) {
        applyFontColor(color);
    });
    
    QObject::connect(settings, &SettingsWidget::bgColorChanged, [=](const QString &color) {
        applyBgColor(color);
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
    updateConnectionStatus();
}

void MenuWidget::loadSettings()
{
    m_serverIP = m_settings->value("serverIP", "127.0.0.1").toString();
    m_serverPort = m_settings->value("serverPort", 9999).toInt();
    m_autoConnect = m_settings->value("autoConnect", true).toBool();
    m_fontColor = m_settings->value("fontColor", "#000000").toString();
    m_bgColor = m_settings->value("bgColor", "#ffffff").toString();
    applyFontColor(m_fontColor);
    applyBgColor(m_bgColor);
}

void MenuWidget::saveSettings()
{
    m_settings->setValue("serverIP", m_serverIP);
    m_settings->setValue("serverPort", m_serverPort);
    m_settings->setValue("autoConnect", m_autoConnect);
}

void MenuWidget::updateConnectionStatus()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        ui->statusLabel->setText("服务器状态：已连接");
        ui->statusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else if (socket->state() == QTcpSocket::ConnectingState) {
        ui->statusLabel->setText("服务器状态：连接中...");
        ui->statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    } else {
        ui->statusLabel->setText("服务器状态：未连接");
        ui->statusLabel->setStyleSheet("color: red; font-weight: bold;");
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
    if (m_autoConnect) {
        connectToServer();
    }
}

void MenuWidget::onDoctorChatBtnClicked()
{
    emit openDoctorChat(m_serverIP, m_serverPort);
}

void MenuWidget::onMemberRechargeBtnClicked()
{
    QMessageBox::information(this, "会员充值", "会员充值功能开发中...");
}
