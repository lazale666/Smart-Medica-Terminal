#include "doctorchatwidget.h"
#include "ui_doctorchatwidget.h"
#include "../Client/chatmessagewidgets.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QTextStream>
#include <QVBoxLayout>
#include <QStringConverter>
#include <QtEndian>
#include <algorithm>

DoctorChatWidget::DoctorChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , m_sessionList(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
    , m_newMessageButton(nullptr)
    , m_currentClientLabel(nullptr)
    , socket(new QTcpSocket(this))
    , m_externalSocket(false)
    , historyDialog(new HistoryDialog(this))
    , m_recordDetailWidget(nullptr)
    , settingsWidget(new SettingsWidget_Doc())
    , m_currentClientName(QStringLiteral("未选择患者"))
    , m_hasActiveClient(false)
    , m_isUserNearBottom(true)
{
    ui->setupUi(this);
    setMinimumSize(1080, 700);
    settingsWidget->setUsername(m_username);
    setupMessageArea();
    initConnections();
    showWelcomeState();
    connectToServer();
}

DoctorChatWidget::DoctorChatWidget(QTcpSocket *existingSocket, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , m_sessionList(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
    , m_newMessageButton(nullptr)
    , m_currentClientLabel(nullptr)
    , socket(existingSocket ? existingSocket : new QTcpSocket(this))
    , m_externalSocket(existingSocket != nullptr)
    , historyDialog(new HistoryDialog(this))
    , m_recordDetailWidget(nullptr)
    , settingsWidget(new SettingsWidget_Doc())
    , m_currentClientName(QStringLiteral("未选择患者"))
    , m_hasActiveClient(false)
    , m_isUserNearBottom(true)
{
    ui->setupUi(this);
    setMinimumSize(1080, 700);
    socket->setParent(this);
    settingsWidget->setUsername(m_username);
    setupMessageArea();
    initConnections();
    showWelcomeState();

    if (socket->state() == QTcpSocket::ConnectedState) {
        setStatusText(QStringLiteral("状态：已连接"), QStringLiteral("#31ffb7"));
    }
}

DoctorChatWidget::~DoctorChatWidget()
{
    for (auto it = m_conversations.keyBegin(); it != m_conversations.keyEnd(); ++it) {
        endHistorySession(*it, false);
    }

    if (settingsWidget) {
        settingsWidget->close();
        delete settingsWidget;
    }
    if (m_recordDetailWidget) {
        m_recordDetailWidget->close();
        delete m_recordDetailWidget;
    }
    delete ui;
}

void DoctorChatWidget::setUsername(const QString &username)
{
    m_username = username;
    settingsWidget->setUsername(username);
}

void DoctorChatWidget::setCredentials(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;
    settingsWidget->setUsername(username);
}

bool DoctorChatWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    Q_UNUSED(event);
    return false;
}

void DoctorChatWidget::setupMessageArea()
{
    m_sessionList = new QListWidget(this);
    m_sessionList->setObjectName("sessionList");
    m_sessionList->setMinimumWidth(280);
    m_sessionList->setMaximumWidth(320);
    m_sessionList->setSpacing(8);

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
    QGridLayout *messageLayerLayout = new QGridLayout(messageLayer);
    messageLayerLayout->setContentsMargins(0, 0, 0, 0);
    messageLayerLayout->setSpacing(0);
    messageLayerLayout->addWidget(m_messageScrollArea, 0, 0);

    QWidget *contentPanel = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentPanel);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(18);
    contentLayout->addWidget(m_sessionList);
    contentLayout->addWidget(messageLayer, 1);

    if (QVBoxLayout *rootLayout = qobject_cast<QVBoxLayout *>(layout())) {
        rootLayout->replaceWidget(ui->textBrowser, contentPanel);
    }
    ui->textBrowser->hide();
    ui->textBrowser->deleteLater();
    ui->textBrowser = nullptr;

    m_newMessageButton = new QPushButton(QStringLiteral("有新消息"), messageLayer);
    m_newMessageButton->setObjectName("newMessageButton");
    m_newMessageButton->setVisible(false);
    messageLayerLayout->addWidget(m_newMessageButton, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);

    connect(m_newMessageButton, &QPushButton::clicked, this, [this]() {
        scrollToBottomAndClearReminder();
    });
    connect(m_sessionList, &QListWidget::itemClicked, this, &DoctorChatWidget::onSessionItemClicked);
    connect(m_messageScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        updateScrollState();
        if (m_isUserNearBottom) {
            updateNewMessageButtonVisibility(false);
        }
    });
}

void DoctorChatWidget::initConnections()
{
    setStyleSheet(R"(
        QWidget#DoctorChatWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111f, stop:0.55 #071b2f, stop:1 #0b1023);
        }
        QWidget#headerWidget, QWidget#inputWidget {
            background: rgba(4, 15, 31, 0.82);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
        }
        QListWidget#sessionList {
            background: rgba(2, 9, 20, 0.86);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
            color: #EAFBFF;
            padding: 10px;
            font: 13px "Microsoft YaHei";
        }
        QListWidget#sessionList::item {
            padding: 10px 10px;
            margin: 4px 0;
            border-radius: 12px;
        }
        QListWidget#sessionList::item:selected {
            background: rgba(0, 229, 255, 0.22);
            border: 1px solid rgba(49, 255, 183, 0.45);
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
            color: #EAFBFF;
            padding: 10px 16px;
            font: 14px "Microsoft YaHei";
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);
            border: 1px solid rgba(234, 251, 255, 0.65);
            border-radius: 16px;
            color: #03111D;
            padding: 8px 18px;
            font: 700 14px "Microsoft YaHei";
            min-height: 42px;
        }
        QPushButton#settingsBtn {
            background: rgba(6, 24, 45, 0.92);
            color: #D8F7FF;
            border: 1px solid rgba(0, 229, 255, 0.75);
        }
        QPushButton#newMessageButton {
            background: rgba(255, 207, 90, 0.95);
            color: #03111D;
            border: 1px solid rgba(255, 122, 89, 0.75);
            border-radius: 14px;
            padding: 6px 14px;
            font: 700 12px "Microsoft YaHei";
        }
        QLabel {
            color: #D8F7FF;
            font-family: "Microsoft YaHei";
        }
        QLabel#titleLabel {
            color: #00E5FF;
            font: 700 24px "Microsoft YaHei";
        }
    )");

    ui->titleLabel->setText(QStringLiteral("医生咨询中心"));
    ui->historyBtn->setText(QStringLiteral("历史记录"));
    ui->settingsBtn->setText(QStringLiteral("设置"));
    ui->sendBtn->setText(QStringLiteral("发送回复"));
    ui->lineEdit->setPlaceholderText(QStringLiteral("输入给当前患者的回复..."));
    setStatusText(QStringLiteral("状态：连接中..."), QStringLiteral("#ffcf5a"));

    auto *viewRecordBtn = new QPushButton(QStringLiteral("查看病历"), ui->headerWidget);
    viewRecordBtn->setObjectName(QStringLiteral("viewRecordBtn"));
    viewRecordBtn->setEnabled(false);

    m_currentClientLabel = new QLabel(QStringLiteral("当前会话：%1").arg(m_currentClientName), this);
    m_currentClientLabel->setStyleSheet("color: #8BB9C8; font-size: 13px; font-weight: 700;");

    if (QHBoxLayout *buttonLayout = qobject_cast<QHBoxLayout *>(ui->headerWidget->findChild<QHBoxLayout *>(QStringLiteral("buttonLayout")))) {
        buttonLayout->insertWidget(1, viewRecordBtn);
        buttonLayout->insertWidget(buttonLayout->count() - 1, m_currentClientLabel);
    }

    connect(socket, &QTcpSocket::connected, this, [this]() {
        setStatusText(QStringLiteral("状态：已连接"), QStringLiteral("#31ffb7"));
    });
    connect(socket, &QTcpSocket::connected, this, &DoctorChatWidget::sendLoginRequest);
    connect(socket, &QTcpSocket::disconnected, this, [this, viewRecordBtn]() {
        setStatusText(QStringLiteral("状态：已断开"), QStringLiteral("#ff5f7e"));
        updateCurrentClientLabel(QStringLiteral("未选择患者"));
        viewRecordBtn->setEnabled(false);
    });
    connect(socket, &QTcpSocket::readyRead, this, &DoctorChatWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, [this]() {
        setStatusText(QStringLiteral("状态：连接失败"), QStringLiteral("#ff5f7e"));
    });

    connect(ui->settingsBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_settingsBtn_clicked);
    connect(ui->historyBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_historyBtn_clicked);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_sendBtn_clicked);
    connect(viewRecordBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_viewRecordBtn_clicked);

    connect(settingsWidget, &SettingsWidget_Doc::serverConfigChanged, this, [this](const QString &, quint16) {
        if (socket->state() == QTcpSocket::ConnectedState || socket->state() == QTcpSocket::ConnectingState) {
            socket->abort();
        }
        connectToServer();
    });
    connect(settingsWidget, &SettingsWidget_Doc::logout, this, [this]() {
        if (settingsWidget) {
            settingsWidget->close();
        }
        close();
    });
}

void DoctorChatWidget::connectToServer()
{
    if (socket->state() == QTcpSocket::ConnectedState || socket->state() == QTcpSocket::ConnectingState) {
        socket->abort();
    }

    QSettings settings("SmartMedica", "DoctorClient");
    const QString ip = settings.value("serverIP", "127.0.0.1").toString();
    const quint16 port = settings.value("serverPort", 9999).toUInt();
    setStatusText(QStringLiteral("状态：连接中..."), QStringLiteral("#ffcf5a"));
    socket->connectToHost(ip, port);
}

void DoctorChatWidget::sendLoginRequest()
{
    if (m_username.trimmed().isEmpty() || socket->state() != QTcpSocket::ConnectedState) {
        return;
    }

    QJsonObject obj;
    obj["type"] = "login";
    obj["username"] = m_username;
    obj["password"] = m_password;
    obj["role"] = "doctor";

    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);
    socket->write(sendData);
    socket->flush();
}

bool DoctorChatWidget::sendMessage(const QString &sessionId, const QString &message)
{
    if (socket->state() != QTcpSocket::ConnectedState) {
        QMessageBox::warning(nullptr, QStringLiteral("未连接服务器"), QStringLiteral("请先在设置中配置并连接服务器。"));
        return false;
    }
    if (sessionId.trimmed().isEmpty() || !m_conversations.contains(sessionId)) {
        QMessageBox::information(nullptr, QStringLiteral("未选择患者"), QStringLiteral("请先从左侧列表选择一个患者会话。"));
        return false;
    }

    if (!m_conversations[sessionId].isOnline) {
        QMessageBox::information(nullptr, QStringLiteral("患者已离线"), QStringLiteral("该患者已断开，请等待患者重新进入名师对话。"));
        return false;
    }

    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = "doctor";
    obj["target_session_id"] = sessionId;

    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);
    socket->write(sendData);
    socket->flush();
    return true;
}

void DoctorChatWidget::showWelcomeState()
{
    if (!m_messages.isEmpty()) {
        return;
    }
    appendSystemMessage(QStringLiteral("欢迎进入医生咨询中心"));
    appendSystemMessage(QStringLiteral("左侧会显示当前连接的患者，每位患者的消息会独立保存。"));
    appendSystemMessage(QStringLiteral("等待患者连接中..."));
}

void DoctorChatWidget::clearWelcomeStateIfNeeded()
{
    if (m_messages.size() == 3 && m_messages[0].isSystem && m_messages[1].isSystem && m_messages[2].isSystem) {
        m_messages.clear();
    }
}

void DoctorChatWidget::updateScrollState()
{
    m_isUserNearBottom = isScrollAreaNearBottom(m_messageScrollArea);
}

void DoctorChatWidget::updateNewMessageButtonVisibility(bool visible)
{
    if (!m_newMessageButton) {
        return;
    }
    m_newMessageButton->setVisible(visible);
    if (visible) {
        m_newMessageButton->raise();
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
    m_currentClientName = clientName.isEmpty() ? QStringLiteral("未选择患者") : clientName;
    if (m_currentClientLabel) {
        m_currentClientLabel->setText(QStringLiteral("当前会话：%1").arg(m_currentClientName));
    }
    if (QPushButton *viewRecordBtn = ui->headerWidget->findChild<QPushButton *>(QStringLiteral("viewRecordBtn"))) {
        viewRecordBtn->setEnabled(m_hasActiveClient && !m_currentClientName.isEmpty() && m_currentClientName != QStringLiteral("未选择患者"));
    }
}

void DoctorChatWidget::setStatusText(const QString &text, const QString &color)
{
    ui->statusLabel->setText(text);
    ui->statusLabel->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: 700;").arg(color));
}

QString DoctorChatWidget::getHistoryDir() const
{
    const QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history_doctor/" + sanitizeUserName(m_username);
    QDir().mkpath(historyDir);
    return historyDir;
}

QString DoctorChatWidget::startHistorySession(const QString &clientName)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"));
    const QString historyFile = QStringLiteral("%1/doctor_chat_%2_%3.txt")
                                    .arg(getHistoryDir(), timestamp, sanitizeUserName(clientName));
    appendHistoryMessage(historyFile, QStringLiteral("system"), QString(), QStringLiteral("会话开始：%1").arg(clientName));
    return historyFile;
}

void DoctorChatWidget::appendHistoryMessage(const QString &historyFile, const QString &role, const QString &sender, const QString &message)
{
    if (historyFile.isEmpty()) {
        return;
    }
    QFile file(historyFile);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    QString safeMessage = message;
    safeMessage.replace("\\", "\\\\");
    safeMessage.replace("\n", "\\n");
    out << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"))
        << "|" << role << "|" << sender << "|" << safeMessage << "\n";
}

void DoctorChatWidget::endHistorySession(const QString &sessionId, bool appendDisconnectedMessage)
{
    auto it = m_conversations.find(sessionId);
    if (it == m_conversations.end() || !it->historyStarted || it->historyFile.isEmpty()) {
        return;
    }
    if (appendDisconnectedMessage) {
        appendHistoryMessage(it->historyFile, QStringLiteral("system"), QString(), QStringLiteral("会话结束"));
    }
    it->historyStarted = false;
}

void DoctorChatWidget::ensureConversationExists(const QString &sessionId, const QString &clientName)
{
    if (sessionId.trimmed().isEmpty()) {
        return;
    }

    QString matchedSessionId;
    for (auto it = m_conversations.begin(); it != m_conversations.end(); ++it) {
        if (it->clientName == clientName && it.key() != sessionId) {
            matchedSessionId = it.key();
            break;
        }
    }

    if (!matchedSessionId.isEmpty()) {
        DoctorConversationState state = m_conversations.take(matchedSessionId);
        state.sessionId = sessionId;
        if (m_activeConversationSessionId == matchedSessionId) {
            m_activeConversationSessionId = sessionId;
        }
        if (!m_conversations.contains(sessionId)) {
            m_conversations.insert(sessionId, state);
        }
    }

    if (!m_conversations.contains(sessionId)) {
        DoctorConversationState state;
        state.sessionId = sessionId;
        state.clientName = clientName;
        state.historyFile = startHistorySession(clientName);
        state.historyStarted = true;
        state.isOnline = true;
        m_conversations.insert(sessionId, state);
    } else {
        DoctorConversationState &state = m_conversations[sessionId];
        state.sessionId = sessionId;
        state.clientName = clientName;
        state.isOnline = true;
        if (!state.historyStarted) {
            state.historyFile = startHistorySession(clientName);
            state.historyStarted = true;
        }
    }
}

void DoctorChatWidget::switchToConversation(const QString &sessionId)
{
    if (!m_conversations.contains(sessionId)) {
        return;
    }

    m_activeConversationSessionId = sessionId;
    DoctorConversationState &state = m_conversations[sessionId];
    m_hasActiveClient = state.isOnline;
    state.unreadCount = 0;
    m_messages = state.messages;
    updateCurrentClientLabel(displayNameForClient(state.clientName));
    rebuildMessages();
    updateSessionList();
}

void DoctorChatWidget::updateSessionList()
{
    if (!m_sessionList) {
        return;
    }

    const QString selectedSession = m_activeConversationSessionId;
    m_sessionList->clear();

    QStringList sessionIds = m_conversations.keys();
    std::sort(sessionIds.begin(), sessionIds.end(), [this](const QString &left, const QString &right) {
        const DoctorConversationState &leftState = m_conversations[left];
        const DoctorConversationState &rightState = m_conversations[right];
        if (leftState.unreadCount != rightState.unreadCount) {
            return leftState.unreadCount > rightState.unreadCount;
        }
        return QString::localeAwareCompare(leftState.clientName, rightState.clientName) < 0;
    });

    for (const QString &sessionId : sessionIds) {
        const DoctorConversationState &state = m_conversations[sessionId];
        QString itemText = displayNameForClient(state.clientName);
        itemText += state.unreadCount > 0
            ? QStringLiteral("\n未读 %1 条").arg(state.unreadCount)
            : QStringLiteral("\n在线会话");
        if (!state.lastPreview.isEmpty()) {
            itemText += QStringLiteral("\n") + summarizePreview(state.lastPreview);
        }

        auto *item = new QListWidgetItem(itemText, m_sessionList);
        item->setData(Qt::UserRole, sessionId);
        if (sessionId == selectedSession) {
            item->setSelected(true);
        }
    }
}

QString DoctorChatWidget::summarizePreview(const QString &message) const
{
    QString preview = message.simplified();
    if (preview.length() > 28) {
        preview = preview.left(28) + "...";
    }
    return preview;
}

QString DoctorChatWidget::displayNameForClient(const QString &clientName) const
{
    return clientName.trimmed().isEmpty() ? QStringLiteral("匿名患者") : clientName;
}

QString DoctorChatWidget::sanitizeUserName(const QString &username) const
{
    QString safeName = username.trimmed();
    if (safeName.isEmpty()) {
        safeName = QStringLiteral("anonymous");
    }
    safeName.replace(QRegularExpression(QStringLiteral(R"([\\/:*?"<>|\s]+)")), QStringLiteral("_"));
    return safeName;
}

QString DoctorChatWidget::recordDirForClient(const QString &clientName) const
{
    return QDir::homePath() + "/SmartMedica/records/" + sanitizeUserName(clientName);
}

QStringList DoctorChatWidget::availableRecordsForClient(const QString &clientName) const
{
    QDir dir(recordDirForClient(clientName));
    if (!dir.exists()) {
        return {};
    }
    return dir.entryList(QStringList() << "record_*.txt", QDir::Files, QDir::Time);
}

bool DoctorChatWidget::loadRecordDataForClient(const QString &clientName, const QString &fileName, QString &diseaseName, QString &diagnosisDate, QString &treatment) const
{
    QFile file(recordDirForClient(clientName) + "/" + fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    diseaseName.clear();
    diagnosisDate.clear();
    treatment.clear();
    bool inTreatment = false;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.startsWith(QStringLiteral("疾病名称:"))) {
            diseaseName = line.mid(QStringLiteral("疾病名称:").size());
        } else if (line.startsWith(QStringLiteral("诊断日期:"))) {
            diagnosisDate = line.mid(QStringLiteral("诊断日期:").size());
        } else if (line.startsWith(QStringLiteral("治疗建议:"))) {
            inTreatment = true;
            treatment = line.mid(QStringLiteral("治疗建议:").size());
        } else if (inTreatment) {
            treatment += "\n" + line;
        }
    }
    return !diseaseName.isEmpty();
}

void DoctorChatWidget::rebuildMessages()
{
    if (!m_messageLayout || !m_messageContent || !m_messageScrollArea) {
        return;
    }

    const bool wasNearBottom = m_isUserNearBottom;
    clearLayoutWidgets(m_messageLayout);
    const ChatThemePalette palette = buildChatThemePalette(false);
    const int maxBubbleWidth = qMax(300, qRound(width() * 0.52));

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

        if (type == "login_success") {
            setWindowTitle(QStringLiteral("医生咨询中心 - %1").arg(obj.value("name").toString()));
        } else if (type == "new_client") {
            clearWelcomeStateIfNeeded();
            QString clientName = obj.value("client_name").toString();
            if (clientName.isEmpty()) {
                clientName = obj.value("username").toString();
            }
            QString sessionId = obj.value("session_id").toString();
            if (sessionId.isEmpty()) {
                sessionId = clientName;
            }

            ensureConversationExists(sessionId, clientName);
            DoctorConversationState &state = m_conversations[sessionId];
            state.messages.append({QString(), QStringLiteral("患者 %1 已连接").arg(clientName), false, true});
            state.lastPreview = QStringLiteral("患者已连接");
            appendHistoryMessage(state.historyFile, QStringLiteral("system"), QString(), QStringLiteral("患者 %1 已连接").arg(clientName));

            if (m_activeConversationSessionId.isEmpty()) {
                switchToConversation(sessionId);
            } else if (m_activeConversationSessionId != sessionId) {
                state.unreadCount += 1;
                updateSessionList();
            } else {
                m_messages = state.messages;
                rebuildMessages();
            }
            setStatusText(QStringLiteral("状态：咨询中"), QStringLiteral("#31ffb7"));
        } else if (type == "client_message") {
            clearWelcomeStateIfNeeded();
            QString clientName = obj.value("client_name").toString();
            if (clientName.isEmpty()) {
                clientName = obj.value("sender").toString();
            }
            QString sessionId = obj.value("session_id").toString();
            if (sessionId.isEmpty()) {
                sessionId = clientName;
            }

            ensureConversationExists(sessionId, clientName);
            DoctorConversationState &state = m_conversations[sessionId];
            const QString message = obj.value("message").toString();
            state.messages.append({displayNameForClient(clientName), message, false, false});
            state.lastPreview = message;
            appendHistoryMessage(state.historyFile, QStringLiteral("client"), clientName, message);

            if (m_activeConversationSessionId == sessionId || m_activeConversationSessionId.isEmpty()) {
                switchToConversation(sessionId);
            } else {
                state.unreadCount += 1;
                updateSessionList();
                updateNewMessageButtonVisibility(true);
            }
        } else if (type == "client_disconnected") {
            QString clientName = obj.value("client_name").toString();
            if (clientName.isEmpty()) {
                clientName = obj.value("username").toString();
            }

            QString sessionId = obj.value("session_id").toString();
            if (sessionId.isEmpty()) {
                for (auto it = m_conversations.begin(); it != m_conversations.end(); ++it) {
                    if (it->clientName == clientName) {
                        sessionId = it.key();
                        break;
                    }
                }
            }
            if (sessionId.isEmpty()) {
                sessionId = clientName;
            }
            if (!m_conversations.contains(sessionId)) {
                continue;
            }

            DoctorConversationState &state = m_conversations[sessionId];
            state.messages.append({QString(), QStringLiteral("患者 %1 已断开").arg(clientName), false, true});
            state.lastPreview = QStringLiteral("患者已断开");
            appendHistoryMessage(state.historyFile, QStringLiteral("system"), QString(), QStringLiteral("患者 %1 已断开").arg(clientName));
            state.isOnline = false;
            endHistorySession(sessionId, true);

            if (m_activeConversationSessionId == sessionId) {
                m_hasActiveClient = false;
                m_messages = state.messages;
                rebuildMessages();
            } else {
                state.unreadCount += 1;
            }
            updateSessionList();
        } else if (type == "doctor_disconnected") {
            appendSystemMessage(QStringLiteral("医生端连接已断开"));
            setStatusText(QStringLiteral("状态：医生端已断开"), QStringLiteral("#ff5f7e"));
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
    if (!sendMessage(m_activeConversationSessionId, message)) {
        return;
    }

    DoctorConversationState &state = m_conversations[m_activeConversationSessionId];
    state.messages.append({m_username.isEmpty() ? QStringLiteral("医生") : m_username, message, true, false});
    state.lastPreview = message;
    appendHistoryMessage(state.historyFile, QStringLiteral("doctor"), m_username, message);

    switchToConversation(m_activeConversationSessionId);
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

void DoctorChatWidget::on_viewRecordBtn_clicked()
{
    if (m_activeConversationSessionId.isEmpty() || !m_conversations.contains(m_activeConversationSessionId)) {
        QMessageBox::information(nullptr, QStringLiteral("未选择患者"), QStringLiteral("请先选择一个正在诊断的患者。"));
        return;
    }

    const QString clientName = m_conversations[m_activeConversationSessionId].clientName;
    const QStringList records = availableRecordsForClient(clientName);
    if (records.isEmpty()) {
        QMessageBox::information(nullptr, QStringLiteral("暂无病历"), QStringLiteral("当前患者暂无可查看的病例。"));
        return;
    }

    QString selectedRecord = records.first();
    if (records.size() > 1) {
        bool ok = false;
        selectedRecord = QInputDialog::getItem(nullptr, QStringLiteral("选择病历"), QStringLiteral("请选择要查看的病历："), records, 0, false, &ok);
        if (!ok || selectedRecord.isEmpty()) {
            return;
        }
    }

    QString diseaseName;
    QString diagnosisDate;
    QString treatment;
    if (!loadRecordDataForClient(clientName, selectedRecord, diseaseName, diagnosisDate, treatment)) {
        QMessageBox::warning(nullptr, QStringLiteral("读取失败"), QStringLiteral("无法读取该患者的病历文件。"));
        return;
    }

    if (m_recordDetailWidget) {
        m_recordDetailWidget->close();
        m_recordDetailWidget->deleteLater();
    }

    m_recordDetailWidget = new RecordDetailWidget(nullptr);
    m_recordDetailWidget->setAttribute(Qt::WA_DeleteOnClose);
    m_recordDetailWidget->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_recordDetailWidget->setWindowTitle(QStringLiteral("病例详情 - %1").arg(displayNameForClient(clientName)));
    m_recordDetailWidget->setRecordData(diseaseName, diagnosisDate, treatment);
    m_recordDetailWidget->setReadOnly(true);
    m_recordDetailWidget->applyAppearance(QStringLiteral("普通模式"), QStringLiteral("#07111F"), QStringLiteral("#D8F7FF"));
    connect(m_recordDetailWidget, &QWidget::destroyed, this, [this]() {
        m_recordDetailWidget = nullptr;
    });
    m_recordDetailWidget->show();
    m_recordDetailWidget->raise();
    m_recordDetailWidget->activateWindow();
}

void DoctorChatWidget::onSessionItemClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    switchToConversation(item->data(Qt::UserRole).toString());
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
