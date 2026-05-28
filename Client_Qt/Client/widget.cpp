#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QListWidgetItem>
#include <QStringConverter>
#include <QSettings>
#include <QFont>
#include <QtEndian>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , settingsWidget(nullptr)
{
    ui->setupUi(this);
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

void Widget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;
    QFont font = ui->textBrowser->font();
    QFont labelFont = ui->userLabel->font();
    QFont btnFont = ui->pushButton->font();
    QFont lineFont = ui->lineEdit->font();

    if (mode == "关怀模式") {
        font.setPointSize(16);
        labelFont.setPointSize(15);
        btnFont.setPointSize(15);
        lineFont.setPointSize(15);

        ui->textBrowser->setFont(font);
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

        ui->textBrowser->setFont(font);
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
    m_fontColor = (color.compare("#000000", Qt::CaseInsensitive) == 0) ? "#D8F7FF" : color;
    ui->textBrowser->setStyleSheet(QString("QTextBrowser { color: %1; background-color: rgba(2, 9, 20, 0.86); border: 1px solid rgba(0, 229, 255, 0.38); border-radius: 16px; padding: 12px; }").arg(m_fontColor));
    ui->lineEdit->setStyleSheet(QString("QLineEdit { color: %1; background-color: rgba(2, 9, 20, 0.86); border: 1px solid rgba(0, 229, 255, 0.55); border-radius: 16px; padding: 8px 12px; }").arg(m_fontColor));
    ui->userLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: 700; }").arg(m_fontColor));
    ui->chatTitleLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: 700; }").arg(m_fontColor));
    ui->voiceLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(m_fontColor));
}

void Widget::applyBgColor(const QString &color)
{
    m_bgColor = (color.compare("#ffffff", Qt::CaseInsensitive) == 0) ? "#07111F" : color;
    applyFontColor(m_fontColor);
    if (m_bgColor.compare("#07111F", Qt::CaseInsensitive) == 0) {
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
        setStyleSheet(QString("QWidget#Widget { background-color: %1; }").arg(m_bgColor));
    }
}

void Widget::connectToServer()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
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

void Widget::createNewChat()
{
    ui->textBrowser->clear();
    m_isNewSession = true;
    m_firstMessage.clear();

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
    ui->textBrowser->clear();
    loadChatHistory(fileName);
    applyFontColor(m_fontColor);
}

void Widget::connectService()
{
    appendSystemMessage("已连接服务器");

    if (!m_currentChatFile.isEmpty()) {
        QFileInfo fi(m_currentChatFile);
        loadChatHistory(fi.fileName());
    }
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

                if (m_isInterrupted) {
                    m_isThinking = false;
                    m_isInterrupted = false;
                    ui->textBrowser->append("已中断");
                    ui->pushButton->setText("发送");
                    continue;
                }

                m_isThinking = false;
                ui->pushButton->setText("发送");
                appendChatMessage("创伤小组", content, false);
                saveChatMessage("创伤小组", content);
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
}

void Widget::onFontColorChanged(const QString &color)
{
    applyFontColor(color);
}

void Widget::onBgColorChanged(const QString &color)
{
    applyBgColor(color);
}

void Widget::onServerConfigChanged(const QString &ip, quint16 port, bool autoConnect)
{
    m_serverIP = ip;
    m_serverPort = port;
    m_autoConnect = autoConnect;

    if (autoConnect) {
        connectToServer();
    }
}

void Widget::onSpeechSettingsChanged(double volume, double rate)
{
    m_speech->setVolume(volume);
    m_speech->setRate(rate);
}

void Widget::onReadBtnClicked()
{
    QString text = ui->textBrowser->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(nullptr, "提示", "没有可朗读的内容");
        return;
    }

    if (m_speech->state() == QTextToSpeech::Speaking) {
        m_speech->stop();
        ui->readBtn->setText("🔊 朗读");
        return;
    }

    ui->textBrowser->append("🔊 正在朗读...");

    QString lastMessage = "";
    QStringList lines = text.split("\n");
    for (int i = lines.size() - 1; i >= 0; i--) {
        QString line = lines[i].trimmed();
        if (line.startsWith("🤖 创伤小组：")) {
            lastMessage = line.mid(5).trimmed();
            break;
        }
    }

    if (lastMessage.isEmpty()) {
        lastMessage = text.right(200).trimmed();
    }

    if (lastMessage.isEmpty()) {
        QMessageBox::information(nullptr, "提示", "没有找到可朗读的内容");
        return;
    }

    QSettings settings("SmartMedica", "Client");
    double volume = settings.value("speechVolume", 1.0).toDouble();
    double rate = settings.value("speechRate", 0.0).toDouble();

    m_speech->setVolume(volume);
    m_speech->setRate(rate);

    ui->readBtn->setText("⏹️ 停止");

    m_speech->say(lastMessage);
    connect(m_speech, &QTextToSpeech::stateChanged, this, [=](QTextToSpeech::State state) {
        if (state == QTextToSpeech::Ready) {
            ui->readBtn->setText("🔊 朗读");
        }
    });
}

void Widget::onCacheCleared()
{
    ui->textBrowser->clear();
    appendSystemMessage("已开启新对话");
    m_isNewSession = true;
    m_firstMessage.clear();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    m_currentChatFile = getHistoryDir() + "/chat_" + timestamp + ".txt";

    refreshHistoryList();
}

void Widget::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
{
    const QString safeSender = sender.toHtmlEscaped();
    const QString safeMessage = message.toHtmlEscaped().replace("\n", "<br>");
    const QString wrapperStyle = isSelf
        ? "margin: 12px 0 12px auto; max-width: 72%; text-align: right;"
        : "margin: 12px auto 12px 0; max-width: 72%; text-align: left;";
    const QString nameColor = isSelf ? "#8BD9FF" : "#31FFB7";
    const QString cardStyle = isSelf
        ? "display:inline-block; background-color: rgba(0,229,255,0.18); border: 1px solid rgba(0,229,255,0.45); border-radius: 16px; padding: 12px 16px; color: #EAFBFF;"
        : "display:inline-block; background-color: rgba(49,255,183,0.14); border: 1px solid rgba(49,255,183,0.35); border-radius: 16px; padding: 12px 16px; color: #EAFBFF;";

    ui->textBrowser->append(QString(
        "<div style=\"%1\">"
        "<div style=\"font-size:12px; font-weight:700; color:%2; margin-bottom:6px;\">%3</div>"
        "<div style=\"%4\">%5</div>"
        "</div>")
        .arg(wrapperStyle, nameColor, safeSender, cardStyle, safeMessage));
}

void Widget::appendSystemMessage(const QString &message)
{
    ui->textBrowser->append(QString(
        "<div style=\"margin: 10px 0; text-align: center;\">"
        "<span style=\"display:inline-block; padding: 6px 14px; border-radius: 14px; "
        "background: rgba(139,185,200,0.14); border: 1px solid rgba(139,185,200,0.28); "
        "color: #8BB9C8; font-size: 12px;\">%1</span>"
        "</div>")
        .arg(message.toHtmlEscaped()));
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
