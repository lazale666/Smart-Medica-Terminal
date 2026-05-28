#include "doctorchatwidget.h"
#include "ui_doctorchatwidget.h"
#include "../Client/chatmessagewidgets.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTextStream>
#include <QVBoxLayout>
#include <QStringConverter>
#include <QtEndian>

DoctorChatWidget::DoctorChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
    , m_newMessageButton(nullptr)
    , m_currentClientLabel(nullptr)
    , m_externalSocket(false)
    , m_currentClientName(QStringLiteral("未连接"))
    , m_isUserNearBottom(true)
{
    ui->setupUi(this);
    setMinimumSize(900, 650);
    socket = new QTcpSocket(this);
    historyDialog = new HistoryDialog(this);
    settingsWidget = new SettingsWidget_Doc();
    setupMessageArea();
    initConnections();
    showWelcomeState();
    connectToServer();
}

DoctorChatWidget::DoctorChatWidget(QTcpSocket *existingSocket, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
    , m_newMessageButton(nullptr)
    , m_currentClientLabel(nullptr)
    , socket(existingSocket)
    , m_externalSocket(true)
    , m_currentClientName(QStringLiteral("未连接"))
    , m_isUserNearBottom(true)
{
    ui->setupUi(this);
    setMinimumSize(900, 650);
    if (socket) {
        socket->setParent(this);
    }
    historyDialog = new HistoryDialog(this);
    settingsWidget = new SettingsWidget_Doc();
    setupMessageArea();
    initConnections();
    showWelcomeState();

    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        setStatusText(QStringLiteral("状态：已连接"), QStringLiteral("#31ffb7"));
    }
}

void DoctorChatWidget::setUsername(const QString &username)
{
    m_username = username;
    settingsWidget->setUsername(username);
}

void DoctorChatWidget::setupMessageArea()
{
    m_messageScrollArea = new QScrollArea(this);
    m_messageScrollArea->setObjectName("messageScrollArea");
    m_messageScrollArea->setWidgetResizable(true);
    m_messageScrollArea->setFrameShape(QFrame::NoFrame);
    m_messageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContent = new QWidget(m_messageScrollArea);
    m_messageContent->setObjectName("messageContent");
    m_messageLayout = new QVBoxLayout(m_messageContent);
    m_messageLayout->setContentsMargins(18, 18, 18, 18);
    m_messageLayout->setSpacing(2);
    m_messageLayout->addStretch();
    m_messageScrollArea->setWidget(m_messageContent);

    QWidget *messageLayer = new QWidget(this);
    messageLayer->setObjectName("messageLayer");
    messageLayer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGridLayout *messageLayerLayout = new QGridLayout(messageLayer);
    messageLayerLayout->setContentsMargins(0, 0, 0, 0);
    messageLayerLayout->setSpacing(0);
    messageLayerLayout->addWidget(m_messageScrollArea, 0, 0);

    QVBoxLayout *rootLayout = qobject_cast<QVBoxLayout *>(layout());
    if (rootLayout) {
        rootLayout->replaceWidget(ui->textBrowser, messageLayer);
        ui->textBrowser->hide();
        ui->textBrowser->deleteLater();
        ui->textBrowser = nullptr;

        m_newMessageButton = new QPushButton(QStringLiteral("有新消息"), messageLayer);
        m_newMessageButton->setObjectName("newMessageButton");
        m_newMessageButton->setVisible(false);
        m_newMessageButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        messageLayerLayout->addWidget(m_newMessageButton, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);
        connect(m_newMessageButton, &QPushButton::clicked, this, [this]() {
            scrollToBottomAndClearReminder();
        });
    }

    connect(m_messageScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        updateScrollState();
        if (m_isUserNearBottom) {
            updateNewMessageButtonVisibility(false);
        }
    });
}

void DoctorChatWidget::initConnections()
{
    setStyleSheet(QString());
    ui->headerWidget->setStyleSheet(QString());
    ui->inputWidget->setStyleSheet(QString());
    if (ui->textBrowser) {
        ui->textBrowser->setStyleSheet(QString());
    }

    setStyleSheet(R"(
        QWidget#DoctorChatWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111f, stop:0.55 #071b2f, stop:1 #0b1023);
        }
        QWidget#headerWidget, QWidget#inputWidget {
            background: rgba(4, 15, 31, 0.82);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
        }
        QLabel {
            color: #d8f7ff;
            font-family: "Microsoft YaHei";
        }
        QLabel#titleLabel {
            color: #00e5ff;
            font: 700 24px "Microsoft YaHei";
        }
        QScrollArea#messageScrollArea {
            background: rgba(2, 9, 20, 0.86);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
        }
        QWidget#messageContent {
            background: transparent;
        }
        QLineEdit {
            background: rgba(2, 9, 20, 0.88);
            border: 1px solid rgba(0, 229, 255, 0.55);
            border-radius: 18px;
            color: #eafbff;
            padding: 10px 16px;
            font: 14px "Microsoft YaHei";
        }
        QLineEdit:focus {
            border: 2px solid #00e5ff;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00e5ff, stop:1 #31ffb7);
            border: 1px solid rgba(234, 251, 255, 0.65);
            border-radius: 16px;
            color: #03111d;
            padding: 8px 18px;
            font: 700 14px "Microsoft YaHei";
            min-height: 42px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31ffb7, stop:1 #00e5ff);
        }
        QPushButton#historyBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFCF5A, stop:1 #FF7A59);
        }
        QPushButton#settingsBtn {
            background: rgba(6, 24, 45, 0.92);
            color: #d8f7ff;
            border: 1px solid rgba(0, 229, 255, 0.75);
        }
        QPushButton#newMessageButton {
            background: rgba(255, 207, 90, 0.95);
            color: #03111D;
            border: 1px solid rgba(255, 122, 89, 0.75);
            border-radius: 14px;
            padding: 6px 14px;
            font: 700 12px "Microsoft YaHei";
            min-width: 96px;
            min-height: 28px;
        }
        QPushButton#newMessageButton:hover {
            background: rgba(255, 122, 89, 1.0);
        }
    )");

    ui->titleLabel->setText(QStringLiteral("医生咨询中心"));
    ui->historyBtn->setText(QStringLiteral("历史记录"));
    ui->settingsBtn->setText(QStringLiteral("设置"));
    ui->sendBtn->setText(QStringLiteral("发送回复"));
    ui->lineEdit->setPlaceholderText(QStringLiteral("输入回复消息..."));
    setStatusText(QStringLiteral("状态：连接中..."), QStringLiteral("#ffcf5a"));

    m_currentClientLabel = new QLabel(QStringLiteral("当前用户：%1").arg(m_currentClientName), this);
    m_currentClientLabel->setObjectName("currentClientLabel");
    m_currentClientLabel->setStyleSheet("color: #8BB9C8; font-size: 13px; font-weight: 700;");
    if (QHBoxLayout *buttonLayout = qobject_cast<QHBoxLayout *>(ui->headerWidget->findChild<QHBoxLayout *>("buttonLayout"))) {
        buttonLayout->insertWidget(buttonLayout->count() - 1, m_currentClientLabel);
    }

    ui->historyBtn->setMinimumSize(128, 42);
    ui->settingsBtn->setMinimumSize(96, 42);
    ui->sendBtn->setMinimumSize(132, 42);
    ui->historyBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->settingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->sendBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->headerWidget->setMinimumHeight(112);

    connect(socket, &QTcpSocket::connected, this, [=]() {
        setStatusText(QStringLiteral("状态：已连接"), QStringLiteral("#31ffb7"));
    });
    connect(socket, &QTcpSocket::disconnected, this, [=]() {
        setStatusText(QStringLiteral("状态：已断开"), QStringLiteral("#ff5f7e"));
        updateCurrentClientLabel(QStringLiteral("未连接"));
    });
    connect(socket, &QTcpSocket::readyRead, this, &DoctorChatWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, [=](QAbstractSocket::SocketError) {
        setStatusText(QStringLiteral("状态：连接失败"), QStringLiteral("#ff5f7e"));
    });

    connect(ui->settingsBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_settingsBtn_clicked);
    connect(ui->historyBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_historyBtn_clicked);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_sendBtn_clicked);
    connect(settingsWidget, &SettingsWidget_Doc::serverConfigChanged, this, [this](const QString &ip, quint16 port) {
        Q_UNUSED(ip);
        Q_UNUSED(port);
        if (socket->state() == QTcpSocket::ConnectedState || socket->state() == QTcpSocket::ConnectingState) {
            socket->abort();
        }
        connectToServer();
    });
    connect(settingsWidget, &SettingsWidget_Doc::logout, this, [=]() {
        close();
    });
}

DoctorChatWidget::~DoctorChatWidget()
{
    if (settingsWidget) {
        settingsWidget->close();
        delete settingsWidget;
        settingsWidget = nullptr;
    }
    delete ui;
}

void DoctorChatWidget::connectToServer()
{
    QSettings settings("SmartMedica", "DoctorClient");
    const QString ip = settings.value("serverIP", "127.0.0.1").toString();
    const quint16 port = settings.value("serverPort", 9999).toUInt();
    socket->connectToHost(ip, port);
}

void DoctorChatWidget::sendMessage(const QString &message)
{
    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = "doctor";

    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);

    socket->write(sendData);
    socket->flush();
}

void DoctorChatWidget::showWelcomeState()
{
    if (!m_messages.isEmpty()) {
        return;
    }

    appendSystemMessage(QStringLiteral("欢迎进入医生咨询中心"));
    appendSystemMessage(QStringLiteral("您将实时接收用户的健康咨询消息"));
    appendSystemMessage(QStringLiteral("等待消息中..."));
}

void DoctorChatWidget::clearWelcomeStateIfNeeded()
{
    if (m_messages.size() == 3
        && m_messages[0].isSystem
        && m_messages[1].isSystem
        && m_messages[2].isSystem) {
        m_messages.clear();
    }
}

void DoctorChatWidget::updateScrollState()
{
    m_isUserNearBottom = isScrollAreaNearBottom(m_messageScrollArea);
}

void DoctorChatWidget::updateNewMessageButtonVisibility(bool visible)
{
    if (m_newMessageButton) {
        m_newMessageButton->setVisible(visible);
        if (visible) {
            m_newMessageButton->raise();
        }
    }
}

void DoctorChatWidget::scrollToBottomAndClearReminder()
{
    scrollAreaToBottom(m_messageScrollArea);
    updateScrollState();
    updateNewMessageButtonVisibility(false);
}

void DoctorChatWidget::updateCurrentClientLabel(const QString &clientName)
{
    m_currentClientName = clientName.isEmpty() ? QStringLiteral("未连接") : clientName;
    if (m_currentClientLabel) {
        m_currentClientLabel->setText(QStringLiteral("当前用户：%1").arg(m_currentClientName));
    }
}

void DoctorChatWidget::setStatusText(const QString &text, const QString &color)
{
    ui->statusLabel->setText(text);
    ui->statusLabel->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 700;").arg(color));
}

QString DoctorChatWidget::getHistoryDir() const
{
    const QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history_doctor";
    QDir dir;
    if (!dir.exists(historyDir)) {
        dir.mkpath(historyDir);
    }
    return historyDir;
}

void DoctorChatWidget::startHistorySession(const QString &clientName)
{
    endHistorySession();

    QString safeClientName = clientName.trimmed();
    if (safeClientName.isEmpty()) {
        safeClientName = QStringLiteral("Unknown");
    }
    safeClientName.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|\\s]+")), QStringLiteral("_"));

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"));
    m_currentHistoryFile = QStringLiteral("%1/doctor_chat_%2_%3.txt")
                               .arg(getHistoryDir(), timestamp, safeClientName);
    appendHistoryMessage(QStringLiteral("system"), QString(), QStringLiteral("会话开始：%1").arg(clientName));
}

void DoctorChatWidget::appendHistoryMessage(const QString &role, const QString &sender, const QString &message)
{
    if (m_currentHistoryFile.isEmpty()) {
        return;
    }

    QFile file(m_currentHistoryFile);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QString safeMessage = message;
    safeMessage.replace("\\", "\\\\");
    safeMessage.replace("\n", "\\n");
    QString safeSender = sender;
    safeSender.replace("\\", "\\\\");
    safeSender.replace("\n", " ");

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"))
        << "|" << role << "|" << safeSender << "|" << safeMessage << "\n";
}

void DoctorChatWidget::endHistorySession()
{
    if (!m_currentHistoryFile.isEmpty()) {
        appendHistoryMessage(QStringLiteral("system"), QString(), QStringLiteral("会话结束"));
        m_currentHistoryFile.clear();
    }
}

void DoctorChatWidget::rebuildMessages()
{
    if (!m_messageLayout || !m_messageContent || !m_messageScrollArea) {
        return;
    }

    const bool wasNearBottom = m_isUserNearBottom;
    clearLayoutWidgets(m_messageLayout);
    const ChatThemePalette palette = buildChatThemePalette(false);
    const int maxBubbleWidth = qMax(300, qRound(width() * 0.68));

    for (const DoctorSideChatMessage &msg : std::as_const(m_messages)) {
        QWidget *item = msg.isSystem
            ? createSystemMessageWidget(msg.message, palette, m_messageContent)
            : createChatMessageWidget(msg.sender, msg.message, msg.isSelf, palette, maxBubbleWidth, m_messageContent);
        m_messageLayout->addWidget(item);
    }

    m_messageLayout->addStretch();
    if (wasNearBottom) {
        scrollToBottomAndClearReminder();
    }
}

void DoctorChatWidget::readData()
{
    m_buffer.append(socket->readAll());

    while (m_buffer.size() >= 4) {
        const quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(m_buffer.constData()));
        if (m_buffer.size() < static_cast<int>(4 + dataLen)) {
            break;
        }

        const QByteArray jsonData = m_buffer.mid(4, dataLen);
        m_buffer = m_buffer.mid(4 + dataLen);

        const QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isObject()) {
            continue;
        }

        const QJsonObject obj = doc.object();
        const QString type = obj.value("type").toString();
        const QString sender = obj.value("sender").toString();

        if (type == "login_success") {
            const QString name = obj.value("name").toString();
            setWindowTitle(QStringLiteral("医生咨询中心 - %1").arg(name));
        } else if (type == "new_client") {
            clearWelcomeStateIfNeeded();
            QString clientName = obj.value("client_name").toString();
            if (clientName.isEmpty()) {
                clientName = obj.value("username").toString();
            }
            if (clientName.isEmpty()) {
                clientName = QStringLiteral("用户");
            }
            updateCurrentClientLabel(clientName);
            startHistorySession(clientName);
            setStatusText(QStringLiteral("状态：咨询中"), QStringLiteral("#31ffb7"));
            appendSystemMessage(QStringLiteral("用户 %1 已连接").arg(clientName));
        } else if (type == "client_message") {
            clearWelcomeStateIfNeeded();
            const QString displaySender = sender.isEmpty() ? m_currentClientName : sender;
            updateCurrentClientLabel(displaySender);
            const QString message = obj.value("message").toString();
            updateScrollState();
            const bool shouldStayPinned = m_isUserNearBottom;
            messageHistory.append(QString("%1：%2").arg(displaySender, message));
            appendChatMessage(displaySender, message, false);
            appendHistoryMessage(QStringLiteral("client"), displaySender, message);
            if (!shouldStayPinned) {
                updateNewMessageButtonVisibility(true);
            }
        } else if (type == "client_disconnected") {
            clearWelcomeStateIfNeeded();
            const QString clientName = obj.value("client_name").toString(m_currentClientName);
            appendHistoryMessage(QStringLiteral("system"), QString(), QStringLiteral("用户 %1 已断开").arg(clientName));
            endHistorySession();
            appendSystemMessage(QStringLiteral("用户 %1 已断开").arg(clientName));
            updateCurrentClientLabel(QStringLiteral("未连接"));
            setStatusText(socket && socket->state() == QTcpSocket::ConnectedState
                              ? QStringLiteral("状态：已连接")
                              : QStringLiteral("状态：已断开"),
                          socket && socket->state() == QTcpSocket::ConnectedState
                              ? QStringLiteral("#31ffb7")
                              : QStringLiteral("#ff5f7e"));
        }
    }
}

void DoctorChatWidget::on_sendBtn_clicked()
{
    const QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    clearWelcomeStateIfNeeded();
    sendMessage(message);
    messageHistory.append(QString("我：%1").arg(message));
    appendHistoryMessage(QStringLiteral("doctor"), m_username.isEmpty() ? QStringLiteral("doctor") : m_username, message);
    appendChatMessage(m_username.isEmpty() ? QStringLiteral("我") : m_username, message, true);
    ui->lineEdit->clear();
    scrollToBottomAndClearReminder();
}

void DoctorChatWidget::on_historyBtn_clicked()
{
    historyDialog->loadHistoryFromDir(getHistoryDir());
    historyDialog->show();
}

void DoctorChatWidget::on_settingsBtn_clicked()
{
    QSettings settings("SmartMedica", "DoctorClient");
    settingsWidget->setServerConfig(settings.value("serverIP", "127.0.0.1").toString(),
                                    settings.value("serverPort", 9999).toUInt());
    settingsWidget->show();
    settingsWidget->raise();
    settingsWidget->activateWindow();
}

void DoctorChatWidget::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
{
    m_messages.append({sender, message, isSelf, false});
    rebuildMessages();
}

void DoctorChatWidget::appendSystemMessage(const QString &message)
{
    m_messages.append({QString(), message, false, true});
    rebuildMessages();
}
