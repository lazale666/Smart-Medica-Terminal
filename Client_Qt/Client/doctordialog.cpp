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

                QString formattedMsg;
                if (sender == m_username) {
                    formattedMsg = QString("<div style='background-color: rgba(0,229,255,0.24); color: #eafbff; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; text-align: right; margin-left: auto; border: 1px solid rgba(0,229,255,0.55);'>\n"
                                          "<span style='font-weight: 600;'>我：</span>%1\n"
                                          "</div>").arg(message.toHtmlEscaped());
                } else {
                    formattedMsg = QString("<div style='background-color: rgba(49,255,183,0.14); color: #eafbff; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; border: 1px solid rgba(49,255,183,0.35);'>\n"
                                          "<span style='font-weight: 600; color: #31ffb7;'>%1：</span>%2\n"
                                          "</div>").arg(m_doctorName.toHtmlEscaped(), message.toHtmlEscaped());
                }
                ui->textBrowser->append(formattedMsg);
            }
        }
    }
}

void DoctorDialog::onSendBtnClicked()
{
    QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) return;

    QString formattedMsg = QString("<div style='background-color: rgba(0,229,255,0.24); color: #eafbff; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; text-align: right; margin-left: auto; border: 1px solid rgba(0,229,255,0.55);'>\n"
                                  "<span style='font-weight: 600;'>我：</span>%1\n"
                                  "</div>").arg(message.toHtmlEscaped());
    ui->textBrowser->append(formattedMsg);

    sendMessage(message);
    ui->lineEdit->clear();
}

void DoctorDialog::onCloseBtnClicked()
{
    close();
}
