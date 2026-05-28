#include "doctordialog.h"
#include "ui_doctordialog.h"
#include <QtEndian>

DoctorDialog::DoctorDialog(QTcpSocket *socket, const QString &username, const QString &doctorName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DoctorDialog)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;
    m_doctorName = doctorName;

    setMinimumSize(640, 520);
    setWindowTitle(QString("与 %1 对话").arg(doctorName));
    ui->lineEdit->setPlaceholderText("输入给医生的消息...");
    ui->sendBtn->setText("发送");
    ui->closeBtn->setText("关闭");
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
            min-width: 72px;
            min-height: 34px;
        }
        QPushButton#closeBtn {
            background: rgba(6, 24, 45, 0.92);
            color: #d8f7ff;
            border: 1px solid rgba(0, 229, 255, 0.65);
        }
    )");

    connect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorDialog::onSendBtnClicked);
    connect(ui->closeBtn, &QPushButton::clicked, this, &DoctorDialog::onCloseBtnClicked);
    appendSystemMessage(QString("已连接医师 %1").arg(m_doctorName));
}

DoctorDialog::~DoctorDialog()
{
    if (m_socket) {
        disconnect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    }
    delete ui;
}

void DoctorDialog::sendMessage(const QString &message)
{
    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = m_username;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
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
        quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(m_buffer.constData()));

        if (m_buffer.size() < static_cast<int>(4 + dataLen)) {
            break;
        }

        QByteArray jsonData = m_buffer.mid(4, dataLen);
        m_buffer = m_buffer.mid(4 + dataLen);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString type = obj.value("type").toString();

            if (type == "client_message") {
                QString sender = obj.value("sender").toString();
                QString message = obj.value("message").toString();
                appendChatMessage(sender == m_username ? m_username : m_doctorName, message, sender == m_username);
            }
        }
    }
}

void DoctorDialog::onSendBtnClicked()
{
    QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) return;

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

    if (mode == "关怀模式") {
        browserFont.setPointSize(16);
        lineFont.setPointSize(15);
        buttonFont.setPointSize(15);

        ui->textBrowser->setFont(browserFont);
        ui->lineEdit->setFont(lineFont);
        ui->sendBtn->setFont(buttonFont);
        ui->closeBtn->setFont(buttonFont);
        ui->lineEdit->setMinimumHeight(52);
        ui->sendBtn->setMinimumHeight(52);
        ui->closeBtn->setMinimumHeight(52);
        resize(900, 680);
    } else {
        browserFont.setPointSize(14);
        lineFont.setPointSize(14);
        buttonFont.setPointSize(14);

        ui->textBrowser->setFont(browserFont);
        ui->lineEdit->setFont(lineFont);
        ui->sendBtn->setFont(buttonFont);
        ui->closeBtn->setFont(buttonFont);
        ui->lineEdit->setMinimumHeight(24);
        ui->sendBtn->setMinimumHeight(34);
        ui->closeBtn->setMinimumHeight(34);
        resize(640, 520);
    }
}

void DoctorDialog::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
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

void DoctorDialog::appendSystemMessage(const QString &message)
{
    ui->textBrowser->append(QString(
        "<div style=\"margin: 10px 0; text-align: center;\">"
        "<span style=\"display:inline-block; padding: 6px 14px; border-radius: 14px; "
        "background: rgba(139,185,200,0.14); border: 1px solid rgba(139,185,200,0.28); "
        "color: #8BB9C8; font-size: 12px;\">%1</span>"
        "</div>")
        .arg(message.toHtmlEscaped()));
}
