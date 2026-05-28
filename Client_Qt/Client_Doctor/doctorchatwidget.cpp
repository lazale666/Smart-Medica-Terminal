#include "doctorchatwidget.h"
#include "ui_doctorchatwidget.h"

DoctorChatWidget::DoctorChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , m_externalSocket(false)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    historyDialog = new HistoryDialog(this);
    initConnections();
    connectToServer();
}

DoctorChatWidget::DoctorChatWidget(QTcpSocket *existingSocket, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , socket(existingSocket)
    , m_externalSocket(true)
{
    ui->setupUi(this);
    historyDialog = new HistoryDialog(this);
    initConnections();
    
    if (socket->state() == QTcpSocket::ConnectedState) {
        ui->statusLabel->setText("状态：已连接");
        ui->statusLabel->setStyleSheet("color: #4CAF50; font-size: 14px;");
    }
}

void DoctorChatWidget::initConnections()
{
    connect(socket, &QTcpSocket::connected, this, [=]() {
        ui->statusLabel->setText("状态：已连接");
        ui->statusLabel->setStyleSheet("color: #4CAF50; font-size: 14px;");
    });
    connect(socket, &QTcpSocket::disconnected, this, [=]() {
        ui->statusLabel->setText("状态：已断开");
        ui->statusLabel->setStyleSheet("color: #f44336; font-size: 14px;");
    });
    connect(socket, &QTcpSocket::readyRead, this, &DoctorChatWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [=](QAbstractSocket::SocketError) {
        ui->statusLabel->setText("状态：连接失败");
        ui->statusLabel->setStyleSheet("color: #f44336; font-size: 14px;");
    });
}

DoctorChatWidget::~DoctorChatWidget()
{
    delete ui;
}

void DoctorChatWidget::connectToServer()
{
    socket->connectToHost("127.0.0.1", 9999);
}

void DoctorChatWidget::sendMessage(const QString &message)
{
    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = "doctor";

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    quint32 len = data.size();
    QByteArray sendData;
    sendData.append((char*)&len, sizeof(quint32));
    sendData.append(data);

    socket->write(sendData);
}

void DoctorChatWidget::readData()
{
    QByteArray buffer;
    while (socket->bytesAvailable() > 0) {
        buffer += socket->readAll();
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
            QString sender = obj.value("sender").toString();

            if (type == "login_success") {
                QString name = obj.value("name").toString();
                setWindowTitle("医生咨询中心 - " + name);
            } else if (type == "new_client") {
                QString clientName = obj.value("client_name").toString();
                ui->textBrowser->append(QString("<div style='color: #999; font-style: italic;'>用户 %1 已连接</div>").arg(clientName));
            } else if (type == "client_message") {
                QString message = obj.value("message").toString();
                messageHistory.append(message);

                QString formattedMsg = QString("<div style='background-color: #f1f8e9; color: #333; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; border: 1px solid #ddd;'>\n"
                                              "<span style='font-weight: 500; color: #388e3c;'>%1：</span>%2\n"
                                              "</div>").arg(sender).arg(message);
                ui->textBrowser->append(formattedMsg);
            }
        }
    }
}

void DoctorChatWidget::on_sendBtn_clicked()
{
    QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) return;

    QJsonObject obj;
    obj["type"] = "message";
    obj["message"] = message;
    obj["sender"] = "doctor";

    QString formattedMsg = QString("<div style='background-color: #00bcd4; color: white; padding: 10px 16px; border-radius: 16px; margin: 10px 0; max-width: 70%; text-align: right; margin-left: auto; border: 1px solid #0097a7;'>\n"
                                  "<span style='font-weight: 500;'>我：</span>%1\n"
                                  "</div>").arg(message);
    ui->textBrowser->append(formattedMsg);

    sendMessage(message);
    ui->lineEdit->clear();
}

void DoctorChatWidget::on_historyBtn_clicked()
{
    QJsonObject obj;
    obj["type"] = "request_history";
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    
    historyDialog->setHistoryData(userHistory);
    historyDialog->exec();
}
