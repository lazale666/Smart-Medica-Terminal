#include "doctordialog.h"
#include "ui_doctordialog.h"

DoctorDialog::DoctorDialog(QTcpSocket *socket, const QString &username, const QString &doctorName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DoctorDialog)
{
    ui->setupUi(this);
    m_socket = socket;
    m_username = username;
    m_doctorName = doctorName;

    setWindowTitle(QString("与 %1 对话").arg(doctorName));

    connect(m_socket, &QTcpSocket::readyRead, this, &DoctorDialog::readData);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorDialog::onSendBtnClicked);
    connect(ui->closeBtn, &QPushButton::clicked, this, &DoctorDialog::onCloseBtnClicked);
}

DoctorDialog::~DoctorDialog()
{
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
    quint32 len = data.size();
    QByteArray sendData;
    sendData.append((char*)&len, sizeof(quint32));
    sendData.append(data);

    m_socket->write(sendData);
}

void DoctorDialog::readData()
{
    QByteArray buffer;
    while (m_socket->bytesAvailable() > 0) {
        buffer += m_socket->readAll();
    }

    while (buffer.size() >= 4) {
        quint32 dataLen;
        memcpy(&dataLen, buffer.constData(), sizeof(quint32));
        dataLen = qFromBigEndian(dataLen);

        if (buffer.size() < 4 + dataLen) {
            break;
        }

        QByteArray jsonData = buffer.mid(4, dataLen);
        buffer = buffer.mid(4 + dataLen);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString type = obj.value("type").toString();

            if (type == "client_message") {
                QString sender = obj.value("sender").toString();
                QString message = obj.value("message").toString();

                QString formattedMsg;
                if (sender == m_username) {
                    formattedMsg = QString("<div style='background-color: #00bcd4; color: white; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; text-align: right; margin-left: auto;'>\n"
                                          "<span style='font-weight: 500;'>我：</span>%1\n"
                                          "</div>").arg(message);
                } else {
                    formattedMsg = QString("<div style='background-color: #f1f8e9; color: #333; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%;'>\n"
                                          "<span style='font-weight: 500; color: #388e3c;'>%1：</span>%2\n"
                                          "</div>").arg(m_doctorName).arg(message);
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

    QString formattedMsg = QString("<div style='background-color: #00bcd4; color: white; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; text-align: right; margin-left: auto;'>\n"
                                  "<span style='font-weight: 500;'>我：</span>%1\n"
                                  "</div>").arg(message);
    ui->textBrowser->append(formattedMsg);

    sendMessage(message);
    ui->lineEdit->clear();
}

void DoctorDialog::onCloseBtnClicked()
{
    close();
}