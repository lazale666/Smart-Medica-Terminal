#include "widget.h"
#include "ui_widget.h"
#include "chatmessagewidgets.h"
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QListWidgetItem>
#include <QStringConverter>
#include <QSettings>
#include <QFont>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QtEndian>
#include "themehelpers.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , settingsWidget(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
    , m_newMessageButton(nullptr)
{
    ui->setupUi(this);
    setupMessageArea();
    socket = new QTcpSocket(this);
    timer = new QTimer(this);
    historyTimer = new QTimer(this);
    msg = new QMessageBox(this);
    dia = new Dialog(this);
    audio = new Audio(this);
    speech = new Speech();
    m_speech = new QTextToSpeech(this);
    msg->close();

    m_settings = new QSettings("SmartMedica", "Client", this);

    m_username = "用户";
    m_isNewSession = true;
    m_autoConnect = true;
    m_currentMode = "普通模式";
    m_fontColor = "#D8F7FF";
    m_bgColor = "#07111F";

    connect(socket, &QTcpSocket::connected, this, &Widget::connectService);
    connect(socket, &QTcpSocket::disconnected, this, &Widget::disConnectService);
    connect(socket, &QTcpSocket::errorOccurred, this, &Widget::connectError);
    connect(timer, &QTimer::timeout, this, &Widget::reconnect);
    connect(historyTimer, &QTimer::timeout, this, &Widget::onHistoryLoadTimerTick);
    connect(ui->historyList, &QListWidget::itemClicked, this, &Widget::onHistoryItemClicked);

    connect(ui->backBtn, &QPushButton::clicked, this, &Widget::onLogout);
    connect(ui->newChatBtn, &QPushButton::clicked, this, [=]() {
        createNewChat();
    });

    connect(ui->settingsBtn, &QPushButton::clicked, this, &Widget::onSettingsBtnClicked);
    connect(ui->readBtn, &QPushButton::clicked, this, &Widget::onReadBtnClicked);

    conFlag = 0;
    errFlag = 0;
    count = 0;
    isRecording = false;
    m_isThinking = false;
    m_isInterrupted = false;
    m_isUserNearBottom = true;
    m_lastAssistantMessage.clear();

    ui->chatTitleLabel->setVisible(true);
    ui->pushButton->setText("发送");

    loadSettings();

    appendSystemMessage("已进入聊天界面");
    appendSystemMessage("可在设置中配置服务器并手动连接");

    createNewChat();
    refreshHistoryList();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::loadSettings()
{
    m_serverIP = m_settings->value("serverIP", "127.0.0.1").toString();
    m_serverPort = m_settings->value("serverPort", 9999).toUInt();
    m_autoConnect = m_settings->value("autoConnect", true).toBool();
    m_currentMode = m_settings->value("mode", "普通模式").toString();
    m_fontColor = m_settings->value("fontColor", "#D8F7FF").toString();
    m_bgColor = m_settings->value("bgColor", "#07111F").toString();

    applyModeSettings(m_currentMode);
    applyBgColor(m_bgColor);
    applyFontColor(m_fontColor);
    scrollToBottomAndClearReminder();
}

void Widget::setUsername(const QString &username)
{
    m_username = username;
    ui->userLabel->setText(username);
}

void Widget::setServerInfo(const QString &ip, int port, bool autoConnect)
{
    m_serverIP = ip;
    m_serverPort = port;
    m_autoConnect = autoConnect;

    if (autoConnect) {
        connectToServer();
    }
}

void Widget::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    applyModeSettings(mode);
    applyBgColor(bgColor);
    applyFontColor(fontColor);
    dia->applyAppearance(m_bgColor, m_fontColor);
}

void Widget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;
    QFont font = m_messageContent ? m_messageContent->font() : ui->lineEdit->font();
    QFont labelFont = ui->userLabel->font();
    QFont btnFont = ui->pushButton->font();
    QFont lineFont = ui->lineEdit->font();

    if (mode == "关怀模式") {
        font.setPointSize(16);
        labelFont.setPointSize(15);
        btnFont.setPointSize(15);
        lineFont.setPointSize(15);

        if (m_messageContent) {
            m_messageContent->setFont(font);
        }
        ui->userLabel->setFont(labelFont);
        ui->pushButton->setFont(btnFont);
        ui->lineEdit->setFont(lineFont);
        ui->voiceBtn->setFont(btnFont);
        ui->newChatBtn->setFont(btnFont);
        ui->settingsBtn->setFont(btnFont);
        ui->readBtn->setFont(btnFont);
        ui->chatTitleLabel->setFont(font);
        ui->historyList->setFont(font);
        ui->voiceLabel->setFont(labelFont);
        ui->backBtn->setFont(btnFont);
        ui->historyTitle->setFont(labelFont);
        ui->historyList->setSpacing(10);
        ui->leftWidget->setMinimumWidth(280);
        ui->leftWidget->setMaximumWidth(320);
        ui->lineEdit->setMinimumHeight(52);
        ui->pushButton->setMinimumHeight(52);
        ui->voiceBtn->setMinimumHeight(52);
        ui->readBtn->setMinimumHeight(52);
        ui->newChatBtn->setMinimumHeight(48);
        ui->backBtn->setMinimumHeight(48);
        ui->settingsBtn->setMinimumHeight(48);
    } else {
        font.setPointSize(10);
        labelFont.setPointSize(10);
        btnFont.setPointSize(10);
        lineFont.setPointSize(10);

        if (m_messageContent) {
            m_messageContent->setFont(font);
        }
        ui->userLabel->setFont(labelFont);
        ui->pushButton->setFont(btnFont);
        ui->lineEdit->setFont(lineFont);
        ui->voiceBtn->setFont(btnFont);
        ui->newChatBtn->setFont(btnFont);
        ui->settingsBtn->setFont(btnFont);
        ui->readBtn->setFont(btnFont);
        ui->chatTitleLabel->setFont(font);
        ui->historyList->setFont(font);
        ui->voiceLabel->setFont(labelFont);
        ui->backBtn->setFont(btnFont);
        ui->historyTitle->setFont(labelFont);
        ui->historyList->setSpacing(4);
        ui->leftWidget->setMinimumWidth(220);
        ui->leftWidget->setMaximumWidth(220);
        ui->lineEdit->setMinimumHeight(36);
        ui->pushButton->setMinimumHeight(36);
        ui->voiceBtn->setMinimumHeight(36);
        ui->readBtn->setMinimumHeight(36);
        ui->newChatBtn->setMinimumHeight(40);
        ui->backBtn->setMinimumHeight(40);
        ui->settingsBtn->setMinimumHeight(36);
    }
}

void Widget::applyFontColor(const QString &color)
{
    m_fontColor = color.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : color;
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    const QString inputBg = light ? "rgba(255, 255, 255, 0.94)" : "rgba(2, 9, 20, 0.86)";
    const QString reminderBg = light ? "rgba(255, 207, 90, 0.96)" : "rgba(255, 207, 90, 0.95)";
    const QString reminderHover = light ? "rgba(255, 180, 90, 1.0)" : "rgba(255, 122, 89, 1.0)";
    if (m_messageScrollArea) {
        m_messageScrollArea->setStyleSheet(QString(
            "QScrollArea#messageScrollArea { background: %1; border: 1px solid rgba(0, 229, 255, 0.38); border-radius: 16px; }"
            "QWidget#messageContent { background: transparent; }").arg(inputBg));
    }
    if (m_newMessageButton) {
        m_newMessageButton->setStyleSheet(QString(
            "QPushButton#newMessageButton { background: %1; color: #03111D; border: 1px solid rgba(255, 122, 89, 0.75); border-radius: 14px; padding: 6px 14px; font: 700 12px \"Microsoft YaHei\"; min-width: 96px; min-height: 28px; }"
            "QPushButton#newMessageButton:hover { background: %2; }").arg(reminderBg, reminderHover));
    }
    ui->lineEdit->setStyleSheet(QString("QLineEdit { color: %1; background-color: %2; border: 1px solid rgba(0, 229, 255, 0.55); border-radius: 16px; padding: 8px 12px; }").arg(m_fontColor, inputBg));
    ui->userLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: 700; }").arg(m_fontColor));
    ui->chatTitleLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: 700; }").arg(m_fontColor));
    ui->voiceLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(m_fontColor));
    rebuildMessages();
}

void Widget::applyBgColor(const QString &color)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(color);
    applyFontColor(m_fontColor);
    if (!ThemeHelpers::isLightTheme(m_bgColor)) {
        setStyleSheet(R"(
            QWidget#Widget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023);
            }
            QWidget#leftWidget {
                background: rgba(4, 15, 31, 0.82);
                border: 1px solid rgba(0, 229, 255, 0.30);
                border-radius: 18px;
            }
            QLabel#historyTitle {
                color: #00E5FF;
                font-weight: 700;
            }
            QListWidget#historyList {
                background: rgba(2, 9, 20, 0.76);
                border: 1px solid rgba(0, 229, 255, 0.28);
                border-radius: 14px;
                color: #EAFBFF;
                padding: 8px;
            }
        )");
    } else {
        setStyleSheet(R"(
            QWidget#Widget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF);
            }
            QWidget#leftWidget {
                background: rgba(255, 255, 255, 0.78);
                border: 1px solid rgba(15, 39, 64, 0.14);
                border-radius: 18px;
            }
            QLabel#historyTitle {
                color: #0F2740;
                font-weight: 700;
            }
            QListWidget#historyList {
                background: rgba(255, 255, 255, 0.92);
                border: 1px solid rgba(15, 39, 64, 0.14);
                border-radius: 14px;
                color: #0F2740;
                padding: 8px;
            }
            QWidget#Widget QFrame#line {
                color: rgba(15, 39, 64, 0.12);
            }
        )");
    }
}

void Widget::connectToServer()
{
    if (socket->state() == QTcpSocket::ConnectedState || socket->state() == QTcpSocket::ConnectingState) {
        socket->abort();
    }
    appendSystemMessage(QString("正在连接服务器 %1:%2...").arg(m_serverIP).arg(m_serverPort));
    socket->connectToHost(m_serverIP, m_serverPort);
    conFlag = 1;
}

QString Widget::getHistoryDir() const
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString historyDir = appDir + "/chat_history";
    QDir dir;
    if (!dir.exists(historyDir)) {
        dir.mkpath(historyDir);
    }
    return historyDir;
}

void Widget::setupMessageArea()
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

    if (QVBoxLayout *rightLayout = qobject_cast<QVBoxLayout *>(ui->rightLayout)) {
        rightLayout->replaceWidget(ui->textBrowser, messageLayer);
    }
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

    connect(m_messageScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        updateScrollState();
        if (m_isUserNearBottom) {
            updateNewMessageButtonVisibility(false);
        }
    });
}

void Widget::clearMessages()
{
    m_messages.clear();
    m_isUserNearBottom = true;
    rebuildMessages();
    updateNewMessageButtonVisibility(false);
}

void Widget::rebuildMessages()
{
    if (!m_messageLayout || !m_messageContent || !m_messageScrollArea) {
        return;
    }

    const bool wasNearBottom = m_isUserNearBottom;
    clearLayoutWidgets(m_messageLayout);
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    const ChatThemePalette palette = buildChatThemePalette(light);
    const int maxBubbleWidth = qMax(280, qRound(width() * 0.58));

    for (const WidgetChatMessage &msg : std::as_const(m_messages)) {
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

void Widget::updateScrollState()
{
    m_isUserNearBottom = isScrollAreaNearBottom(m_messageScrollArea);
}

void Widget::updateNewMessageButtonVisibility(bool visible)
{
    if (m_newMessageButton) {
        m_newMessageButton->setVisible(visible);
        if (visible) {
            m_newMessageButton->raise();
        }
    }
}

void Widget::scrollToBottomAndClearReminder()
{
    scrollAreaToBottom(m_messageScrollArea);
    updateScrollState();
    updateNewMessageButtonVisibility(false);
}

void Widget::createNewChat()
{
    clearMessages();
    m_isNewSession = true;
    m_firstMessage.clear();
    m_lastAssistantMessage.clear();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    m_currentChatFile = getHistoryDir() + "/chat_" + timestamp + ".txt";

    appendSystemMessage("已开启新对话");
    applyFontColor(m_fontColor);
}

void Widget::refreshHistoryList()
{
    ui->historyList->clear();
    QDir dir(getHistoryDir());
    QStringList filters;
    filters << "chat_*.txt";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);

    for (const QFileInfo &fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            QString firstLine = in.readLine();
            file.close();

            if (!firstLine.isEmpty()) {
                QStringList parts = firstLine.split("|");
                QString displayText;
                if (parts.size() >= 3 && parts[1] == "我") {
                    displayText = parts[2].left(20);
                    if (parts[2].length() > 20) displayText += "...";
                } else {
                    displayText = "新对话";
                }
                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, fileInfo.fileName());
                ui->historyList->addItem(item);
            }
        }
    }
}

void Widget::onHistoryItemClicked(QListWidgetItem *item)
{
    QString fileName = item->data(Qt::UserRole).toString();
    m_currentChatFile = getHistoryDir() + "/" + fileName;
    m_isNewSession = false;
    clearMessages();
    loadChatHistory(fileName);
    applyFontColor(m_fontColor);
}

void Widget::connectService()
{
    appendSystemMessage("已连接服务器");
}

void Widget::readData()
{
    buffer.append(socket->readAll());

    while (true) {
        if (buffer.size() < 4) break;

        quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<uchar*>(buffer.data()));
        qint32 totalLen = 4 + dataLen;

        if (buffer.size() < totalLen) break;

        QByteArray jsonData = buffer.mid(4, dataLen);
        buffer = buffer.mid(totalLen);

        QJsonParseError parseErr;
        QJsonDocument docu = QJsonDocument::fromJson(jsonData, &parseErr);
        if (parseErr.error == QJsonParseError::NoError) {
            QJsonObject obj = docu.object();
            QString type = obj.value("type").toString();
            QJsonValue value;

            if (type == "ai_response")
                value = obj.value("data");
            else if (type == "message")
                value = obj.value("message");

            if (value.isString()) {
                QString content = value.toString();
                updateScrollState();
                const bool shouldStayPinned = m_isUserNearBottom;

                if (m_isInterrupted) {
                    m_isThinking = false;
                    m_isInterrupted = false;
                    appendSystemMessage("已中断");
                    ui->pushButton->setText("发送");
                    continue;
                }

                m_isThinking = false;
                ui->pushButton->setText("发送");
                m_lastAssistantMessage = content;
                appendChatMessage("创伤小组", content, false);
                saveChatMessage("创伤小组", content);
                if (!shouldStayPinned) {
                    updateNewMessageButtonVisibility(true);
                }
            }
        }
    }
}

void Widget::disConnectService()
{
    conFlag = 0;
    appendSystemMessage("已断开服务器");
    m_isThinking = false;
    m_isInterrupted = false;
    ui->pushButton->setText("发送");
}

void Widget::connectError(QAbstractSocket::SocketError err)
{
    if (!errFlag) {
        int btn = QMessageBox::warning(nullptr, "网络错误", "服务器错误:" + QString::number(err), QMessageBox::Ok | QMessageBox::Close);
        if (btn == QMessageBox::Ok) {
            if (conFlag) {
                errFlag = 1;
                timer->start(1000);
                while (!dia->exec());
            }
        } else {
            this->close();
        }
    }
}

void Widget::reconnect()
{
    count++;
    socket->connectToHost(m_serverIP, m_serverPort);
}

void Widget::on_pushButton_clicked()
{
    if (m_isThinking) {
        m_isInterrupted = true;
        return;
    }

    QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) return;

    appendChatMessage(m_username.isEmpty() ? "我" : m_username, message, true);
    saveChatMessage("我", message);
    scrollToBottomAndClearReminder();

    if (m_isNewSession && m_firstMessage.isEmpty()) {
        m_firstMessage = message;
        m_isNewSession = false;
        refreshHistoryList();
    }

    QJsonObject obj;
    obj["type"] = "message";
    obj["data"] = message;

    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint32)jsonData.size();
    packet.append(jsonData);

    socket->write(packet);
    socket->flush();
    ui->lineEdit->clear();

    appendSystemMessage("思考中...");
    ui->pushButton->setText("中断");
    m_isThinking = true;
    m_isInterrupted = false;
}

void Widget::on_voiceBtn_pressed()
{
    isRecording = true;
    ui->voiceBtn->setText("🔴");
    ui->voiceLabel->setText("录制中...松开停止");
    audio->startAudioRecord("record.wav");
    appendSystemMessage("开始录音...");
}

void Widget::on_voiceBtn_released()
{
    if (!isRecording) return;

    isRecording = false;
    audio->stopAudioRecord();
    ui->voiceBtn->setText("🎤");
    ui->voiceLabel->setText("按住说话，松开发送");
    appendSystemMessage("识别中...");

    QString recognizedText = speech->speechIdentify("record.wav");
    if (!recognizedText.isEmpty()) {
        appendChatMessage(m_username.isEmpty() ? "我" : m_username, recognizedText, true);
        saveChatMessage("我", recognizedText);
        scrollToBottomAndClearReminder();

        if (m_isNewSession && m_firstMessage.isEmpty()) {
            m_firstMessage = recognizedText;
            m_isNewSession = false;
            refreshHistoryList();
        }

        QJsonObject obj;
        obj["type"] = "message";
        obj["data"] = recognizedText;
        QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << (quint32)jsonData.size();
        packet.append(jsonData);
        socket->write(packet);
        socket->flush();

        appendSystemMessage("思考中...");
        ui->pushButton->setText("中断");
        m_isThinking = true;
        m_isInterrupted = false;
    } else {
        appendSystemMessage("识别失败");
    }
}

void Widget::onHistoryLoadTimerTick()
{
    static int cnt = 0;
    cnt++;
    if (cnt > 100) {
        historyTimer->stop();
        cnt = 0;
    }
}

void Widget::saveChatMessage(const QString &role, const QString &content)
{
    QFile file(m_currentChatFile);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString escapedContent = content;
        escapedContent.replace("\\", "\\\\");
        escapedContent.replace("\n", "\\n");
        out << time << "|" << role << "|" << escapedContent << "\n";
        file.close();
    }
}

void Widget::loadChatHistory(const QString &fileName)
{
    QString fullPath = getHistoryDir() + "/" + fileName;
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    appendHistorySeparator(true);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList parts = line.split("|");
        if (parts.size() >= 3) {
            QString role = parts[1];
            QString content = parts[2];
            content.replace("\\n", "\n");
            content.replace("\\\\", "\\");
            if (role == "我")
                appendChatMessage(m_username.isEmpty() ? "我" : m_username, content, true);
            else
                appendChatMessage("创伤小组", content, false);
        }
    }
    appendHistorySeparator(false);
    file.close();
}

void Widget::onSettingsBtnClicked()
{
    if (!settingsWidget) {
        settingsWidget = new SettingsWidget(nullptr);
        settingsWidget->setWindowModality(Qt::NonModal);
        settingsWidget->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        settingsWidget->setUsername(m_username);
        settingsWidget->setServerConfig(m_serverIP, m_serverPort, m_autoConnect);
        settingsWidget->setCurrentMode(m_currentMode);
        settingsWidget->setFontColor(m_fontColor);
        settingsWidget->setBgColor(m_bgColor);

        connect(settingsWidget, &SettingsWidget::logout, this, &Widget::onLogoutFromSettings);
        connect(settingsWidget, &SettingsWidget::modeChanged, this, &Widget::onModeChanged);
        connect(settingsWidget, &SettingsWidget::fontColorChanged, this, &Widget::onFontColorChanged);
        connect(settingsWidget, &SettingsWidget::bgColorChanged, this, &Widget::onBgColorChanged);
        connect(settingsWidget, &SettingsWidget::serverConfigChanged, this, &Widget::onServerConfigChanged);
        connect(settingsWidget, &SettingsWidget::closeSettings, this, &Widget::onCloseSettings);
        connect(settingsWidget, &SettingsWidget::cacheCleared, this, &Widget::onCacheCleared);
        connect(settingsWidget, &SettingsWidget::speechSettingsChanged, this, &Widget::onSpeechSettingsChanged);
        connect(settingsWidget, &SettingsWidget::destroyed, this, [=]() {
            settingsWidget = nullptr;
        });
    }
    settingsWidget->show();
    settingsWidget->raise();
    settingsWidget->activateWindow();
}

void Widget::onLogout()
{
    leaveChatScene([this]() {
        emit backToMenu();
    });
}

void Widget::onLogoutFromSettings()
{
    leaveChatScene([this]() {
        emit logout();
    });
}

void Widget::onCloseSettings()
{
    if (settingsWidget) {
        SettingsWidget *widget = settingsWidget;
        settingsWidget = nullptr;
        widget->close();
        widget->deleteLater();
    }
}

void Widget::onModeChanged(const QString &mode)
{
    applyModeSettings(mode);
    refreshHistoryList();
    emit modeChanged(mode);
    emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
}

void Widget::onFontColorChanged(const QString &color)
{
    applyFontColor(color);
    emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
}

void Widget::onBgColorChanged(const QString &color)
{
    applyBgColor(color);
    emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
}

void Widget::onServerConfigChanged(const QString &ip, quint16 port, bool autoConnect)
{
    m_serverIP = ip;
    m_serverPort = port;
    m_autoConnect = autoConnect;

    connectToServer();
}

void Widget::onSpeechSettingsChanged(double volume, double rate)
{
    m_speech->setVolume(volume);
    m_speech->setRate(rate);
}

void Widget::onReadBtnClicked()
{
    if (m_lastAssistantMessage.trimmed().isEmpty()) {
        QMessageBox::information(nullptr, "提示", "没有可朗读的内容");
        return;
    }

    if (m_speech->state() == QTextToSpeech::Speaking) {
        m_speech->stop();
        ui->readBtn->setText("🔊 朗读");
        return;
    }

    appendSystemMessage("正在朗读...");

    QSettings settings("SmartMedica", "Client");
    double volume = settings.value("speechVolume", 1.0).toDouble();
    double rate = settings.value("speechRate", 0.0).toDouble();

    m_speech->setVolume(volume);
    m_speech->setRate(rate);

    ui->readBtn->setText("⏹️ 停止");

    m_speech->say(m_lastAssistantMessage);
    connect(m_speech, &QTextToSpeech::stateChanged, this, [=](QTextToSpeech::State state) {
        if (state == QTextToSpeech::Ready) {
            ui->readBtn->setText("🔊 朗读");
        }
    });
}

void Widget::onCacheCleared()
{
    clearMessages();
    appendSystemMessage("已开启新对话");
    m_isNewSession = true;
    m_firstMessage.clear();
    m_lastAssistantMessage.clear();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    m_currentChatFile = getHistoryDir() + "/chat_" + timestamp + ".txt";

    refreshHistoryList();
    scrollToBottomAndClearReminder();
}

void Widget::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
{
    m_messages.append({sender, message, isSelf, false});
    rebuildMessages();
}

void Widget::appendSystemMessage(const QString &message)
{
    m_messages.append({QString(), message, false, true});
    rebuildMessages();
}

void Widget::appendHistorySeparator(bool isTop)
{
    appendSystemMessage(isTop ? "历史对话" : "历史对话结束");
}

void Widget::setSocketHandlersActive(bool active)
{
    if (active) {
        connect(socket, &QTcpSocket::readyRead, this, &Widget::readData, Qt::UniqueConnection);
    } else {
        disconnect(socket, &QTcpSocket::readyRead, this, &Widget::readData);
    }
}

void Widget::leaveChatScene(const std::function<void()> &afterCleanup)
{
    timer->stop();
    historyTimer->stop();
    m_isThinking = false;
    m_isInterrupted = false;
    conFlag = 0;
    errFlag = 0;

    setSocketHandlersActive(false);
    disconnect(socket, &QTcpSocket::connected, this, &Widget::connectService);
    disconnect(socket, &QTcpSocket::disconnected, this, &Widget::disConnectService);
    disconnect(socket, &QTcpSocket::errorOccurred, this, &Widget::connectError);

    if (socket->state() == QTcpSocket::ConnectedState ||
        socket->state() == QTcpSocket::ConnectingState) {
        socket->abort();
    }

    if (settingsWidget) {
        SettingsWidget *widget = settingsWidget;
        settingsWidget = nullptr;
        widget->close();
        widget->deleteLater();
    }

    connect(socket, &QTcpSocket::connected, this, &Widget::connectService, Qt::UniqueConnection);
    connect(socket, &QTcpSocket::disconnected, this, &Widget::disConnectService, Qt::UniqueConnection);
    connect(socket, &QTcpSocket::errorOccurred, this, &Widget::connectError, Qt::UniqueConnection);
    setSocketHandlersActive(true);

    if (afterCleanup) {
        afterCleanup();
    }
}
