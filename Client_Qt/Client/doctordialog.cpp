#include "doctordialog.h"
#include "ui_doctordialog.h"
#include "themehelpers.h"

#include <QtEndian>

DoctorDialog::DoctorDialog(QTcpSocket *socket, const QString &username, const QString &doctorName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DoctorDialog)
    , m_socket(socket)
    , m_username(username)
    , m_doctorName(doctorName)
    , m_currentMode("普通模式")
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
{
    ui->setupUi(this);
    setMinimumSize(640, 520);
    setWindowTitle(QStringLiteral("与 %1 对话").arg(doctorName));
    ui->lineEdit->setPlaceholderText(QStringLiteral("输入给医师的消息..."));

    connect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorDialog::onSendBtnClicked);
    connect(ui->closeBtn, &QPushButton::clicked, this, &DoctorDialog::onCloseBtnClicked);

    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
    appendSystemMessage(QStringLiteral("已连接医师 %1").arg(m_doctorName));
}

DoctorDialog::~DoctorDialog()
{
    if (m_socket) {
        disconnect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    }
    delete ui;
}

void DoctorDialog::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(bgColor);
    m_fontColor = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : fontColor;
    applyModeSettings(mode);

    if (ThemeHelpers::isLightTheme(m_bgColor)) {
        setStyleSheet(R"(
            QDialog#DoctorDialog {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF);
            }
            QTextBrowser {
                background: rgba(255, 255, 255, 0.94);
                border: 1px solid rgba(15, 39, 64, 0.14);
                border-radius: 18px;
                color: #0F2740;
                padding: 18px;
                font: 14px "Microsoft YaHei";
            }
            QLineEdit {
                background: rgba(255, 255, 255, 0.96);
                border: 1px solid rgba(15, 39, 64, 0.18);
                border-radius: 18px;
                color: #0F2740;
                padding: 10px 16px;
                font: 14px "Microsoft YaHei";
                min-height: 24px;
            }
        )");
    } else {
        setStyleSheet(R"(
            QDialog#DoctorDialog {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111f, stop:0.55 #071b2f, stop:1 #0b1023);
            }
            QTextBrowser {
                background: rgba(2, 9, 20, 0.88);
                border: 1px solid rgba(0, 229, 255, 0.42);
                border-radius: 18px;
                color: #eafbff;
                padding: 18px;
                font: 14px "Microsoft YaHei";
            }
            QLineEdit {
                background: rgba(2, 9, 20, 0.88);
                border: 1px solid rgba(0, 229, 255, 0.55);
                border-radius: 18px;
                color: #eafbff;
                padding: 10px 16px;
                font: 14px "Microsoft YaHei";
                min-height: 24px;
            }
        )");
    }
}

void DoctorDialog::sendMessage(const QString &message)
{
    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = m_username;

    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);

    m_socket->write(sendData);
    m_socket->flush();
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
        if (obj.value("type").toString() == "client_message") {
            const QString sender = obj.value("sender").toString();
            const QString message = obj.value("message").toString();
            appendChatMessage(sender == m_username ? m_username : m_doctorName, message, sender == m_username);
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
}

void DoctorDialog::onCloseBtnClicked()
{
    close();
}

void DoctorDialog::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    QFont browserFont = ui->textBrowser->font();
    QFont lineFont = ui->lineEdit->font();
    QFont buttonFont = ui->sendBtn->font();

    if (mode == QStringLiteral("关怀模式")) {
        browserFont.setPointSize(16);
        lineFont.setPointSize(15);
        buttonFont.setPointSize(15);
        ui->lineEdit->setMinimumHeight(52);
        ui->sendBtn->setMinimumHeight(52);
        ui->closeBtn->setMinimumHeight(52);
        resize(900, 680);
    } else {
        browserFont.setPointSize(14);
        lineFont.setPointSize(14);
        buttonFont.setPointSize(14);
        ui->lineEdit->setMinimumHeight(24);
        ui->sendBtn->setMinimumHeight(34);
        ui->closeBtn->setMinimumHeight(34);
        resize(640, 520);
    }

    ui->textBrowser->setFont(browserFont);
    ui->lineEdit->setFont(lineFont);
    ui->sendBtn->setFont(buttonFont);
    ui->closeBtn->setFont(buttonFont);
}

void DoctorDialog::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
{
    const QString safeSender = sender.toHtmlEscaped();
    const QString safeMessage = message.toHtmlEscaped().replace("\n", "<br>");
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    const QString wrapperStyle = isSelf
        ? "margin: 12px 0 12px auto; max-width: 72%; text-align: right;"
        : "margin: 12px auto 12px 0; max-width: 72%; text-align: left;";
    const QString nameColor = isSelf ? (light ? "#0F78B7" : "#8BD9FF") : (light ? "#157A52" : "#31FFB7");
    const QString cardStyle = isSelf
        ? QString("display:inline-block; background-color: %1; border: 1px solid %2; border-radius: 16px; padding: 12px 16px; color: %3;")
              .arg(light ? "rgba(127,217,255,0.35)" : "rgba(0,229,255,0.18)",
                   light ? "rgba(15,120,183,0.35)" : "rgba(0,229,255,0.45)",
                   light ? "#0F2740" : "#EAFBFF")
        : QString("display:inline-block; background-color: %1; border: 1px solid %2; border-radius: 16px; padding: 12px 16px; color: %3;")
              .arg(light ? "rgba(196,240,214,0.65)" : "rgba(49,255,183,0.14)",
                   light ? "rgba(21,122,82,0.28)" : "rgba(49,255,183,0.35)",
                   light ? "#0F2740" : "#EAFBFF");

    ui->textBrowser->append(QString(
        "<div style=\"%1\">"
        "<div style=\"font-size:12px; font-weight:700; color:%2; margin-bottom:6px;\">%3</div>"
        "<div style=\"%4\">%5</div>"
        "</div>")
        .arg(wrapperStyle, nameColor, safeSender, cardStyle, safeMessage));
}

void DoctorDialog::appendSystemMessage(const QString &message)
{
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    ui->textBrowser->append(QString(
        "<div style=\"margin: 10px 0; text-align: center;\">"
        "<span style=\"display:inline-block; padding: 6px 14px; border-radius: 14px; background: %1; border: 1px solid %2; color: %3; font-size: 12px;\">%4</span>"
        "</div>")
        .arg(light ? "rgba(15,39,64,0.08)" : "rgba(139,185,200,0.14)",
             light ? "rgba(15,39,64,0.18)" : "rgba(139,185,200,0.28)",
             light ? "#4C647A" : "#8BB9C8",
             message.toHtmlEscaped()));
}
