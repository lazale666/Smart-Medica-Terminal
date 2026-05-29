#include "doctordialog.h"
#include "ui_doctordialog.h"
#include "themehelpers.h"
#include "chatmessagewidgets.h"

#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtEndian>

DoctorDialog::DoctorDialog(QTcpSocket *socket, const QString &username, const QString &doctorName,
                           const QString &sessionId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DoctorDialog)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
    , m_newMessageButton(nullptr)
    , m_socket(socket)
    , m_username(username)
    , m_doctorName(doctorName)
    , m_sessionId(sessionId)
    , m_currentMode("普通模式")
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
    , m_isUserNearBottom(true)
{
    ui->setupUi(this);
    setMinimumSize(640, 520);
    setWindowTitle(QStringLiteral("与 %1 对话").arg(doctorName));
    ui->lineEdit->setPlaceholderText(QStringLiteral("输入给医生的消息..."));
    ui->sendBtn->setText(QStringLiteral("发送"));
    ui->closeBtn->setText(QStringLiteral("关闭"));

    setupMessageArea();

    connect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorDialog::onSendBtnClicked);
    connect(ui->closeBtn, &QPushButton::clicked, this, &DoctorDialog::onCloseBtnClicked);

    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
    appendSystemMessage(QStringLiteral("已连接医生 %1").arg(m_doctorName));
}

DoctorDialog::~DoctorDialog()
{
    if (m_socket) {
        disconnect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    }
    delete ui;
}

void DoctorDialog::setupMessageArea()
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

void DoctorDialog::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(bgColor);
    m_fontColor = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : fontColor;
    applyModeSettings(mode);

    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    const ChatThemePalette palette = buildChatThemePalette(light);

    setStyleSheet(QString(
        "QDialog#DoctorDialog { background: %1; }"
        "QScrollArea#messageScrollArea { background: %2; border: 1px solid %3; border-radius: 18px; }"
        "QWidget#messageContent { background: transparent; }"
        "QLineEdit { background: %4; border: 1px solid %5; border-radius: 18px; color: %6; padding: 10px 16px; font: 14px \"Microsoft YaHei\"; }"
        "QLineEdit:focus { border: 2px solid #00E5FF; }"
        "QPushButton { background: %7; border: 1px solid %5; border-radius: 16px; color: %8; padding: 8px 18px; font: 700 14px \"Microsoft YaHei\"; min-width: 72px; min-height: 34px; }"
        "QPushButton:hover { background: %9; }"
        "QPushButton#closeBtn { background: %10; color: %11; }"
        "QPushButton#newMessageButton { background: %12; color: %13; border: 1px solid %14; border-radius: 14px; padding: 6px 14px; font: 700 12px \"Microsoft YaHei\"; min-width: 96px; min-height: 28px; }"
        "QPushButton#newMessageButton:hover { background: %15; }")
                      .arg(palette.pageBackground,
                           light ? "rgba(255, 255, 255, 0.94)" : "rgba(2, 9, 20, 0.88)",
                           light ? "rgba(15, 39, 64, 0.14)" : "rgba(0, 229, 255, 0.42)",
                           palette.inputBackground,
                           palette.inputBorder,
                           palette.mainText,
                           "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7)",
                           "#03111D",
                           "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31FFB7, stop:1 #00E5FF)",
                           palette.buttonBackground,
                           palette.buttonText,
                           "rgba(255, 207, 90, 0.95)",
                           "#03111D",
                           "rgba(255, 122, 89, 0.75)",
                           "rgba(255, 122, 89, 1.0)"));

    rebuildMessages();
}

void DoctorDialog::sendMessage(const QString &message)
{
    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = m_username;
    obj["session_id"] = m_sessionId;

    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);

    m_socket->write(sendData);
    m_socket->flush();
}

void DoctorDialog::updateScrollState()
{
    m_isUserNearBottom = isScrollAreaNearBottom(m_messageScrollArea);
}

void DoctorDialog::updateNewMessageButtonVisibility(bool visible)
{
    if (m_newMessageButton) {
        m_newMessageButton->setVisible(visible);
        if (visible) {
            m_newMessageButton->raise();
        }
    }
}

void DoctorDialog::scrollToBottomAndClearReminder()
{
    scrollAreaToBottom(m_messageScrollArea);
    updateScrollState();
    updateNewMessageButtonVisibility(false);
}

void DoctorDialog::rebuildMessages()
{
    if (!m_messageLayout || !m_messageContent || !m_messageScrollArea) {
        return;
    }

    const bool wasNearBottom = m_isUserNearBottom;
    clearLayoutWidgets(m_messageLayout);
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    const ChatThemePalette palette = buildChatThemePalette(light);
    const int maxBubbleWidth = qMax(260, qRound(width() * 0.66));

    for (const DoctorChatMessage &msg : std::as_const(m_messages)) {
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

void DoctorDialog::readData()
{
    m_buffer.append(m_socket->readAll());

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
        if (type == "client_message") {
            const QString sender = obj.value("sender").toString();
            const QString message = obj.value("message").toString();
            updateScrollState();
            const bool shouldStayPinned = m_isUserNearBottom;
            appendChatMessage(sender == m_username ? m_username : m_doctorName, message, sender == m_username);
            if (!shouldStayPinned && sender != m_username) {
                updateNewMessageButtonVisibility(true);
            }
        } else if (type == "doctor_disconnected") {
            appendSystemMessage(QStringLiteral("医生已断开，请返回重新选择医生。"));
        } else if (type == "connection_failed") {
            const QString message = obj.value("message").toString(QStringLiteral("连接已失效，请返回重新连接医生。"));
            appendSystemMessage(message);
        }
    }
}

void DoctorDialog::onSendBtnClicked()
{
    const QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    appendChatMessage(m_username, message, true);
    sendMessage(message);
    ui->lineEdit->clear();
    scrollToBottomAndClearReminder();
}

void DoctorDialog::onCloseBtnClicked()
{
    close();
}

void DoctorDialog::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    QFont lineFont = ui->lineEdit->font();
    QFont buttonFont = ui->sendBtn->font();

    if (mode == QStringLiteral("关怀模式")) {
        lineFont.setPointSize(15);
        buttonFont.setPointSize(15);
        ui->lineEdit->setMinimumHeight(52);
        ui->sendBtn->setMinimumHeight(52);
        ui->closeBtn->setMinimumHeight(52);
        resize(900, 680);
    } else {
        lineFont.setPointSize(14);
        buttonFont.setPointSize(14);
        ui->lineEdit->setMinimumHeight(38);
        ui->sendBtn->setMinimumHeight(38);
        ui->closeBtn->setMinimumHeight(38);
        resize(640, 520);
    }

    ui->lineEdit->setFont(lineFont);
    ui->sendBtn->setFont(buttonFont);
    ui->closeBtn->setFont(buttonFont);
    if (m_newMessageButton) {
        m_newMessageButton->setFont(QFont(QStringLiteral("Microsoft YaHei"), mode == QStringLiteral("关怀模式") ? 11 : 10, QFont::Bold));
    }
}

void DoctorDialog::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
{
    m_messages.append({sender, message, isSelf, false});
    rebuildMessages();
}

void DoctorDialog::appendSystemMessage(const QString &message)
{
    m_messages.append({QString(), message, false, true});
    rebuildMessages();
}
