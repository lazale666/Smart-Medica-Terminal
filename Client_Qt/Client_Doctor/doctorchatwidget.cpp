#include "doctorchatwidget.h"
#include "ui_doctorchatwidget.h"
#include <QSettings>
#include <QPushButton>
#include <QSizePolicy>
#include <QtEndian>

DoctorChatWidget::DoctorChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorChatWidget)
    , m_externalSocket(false)
{
    ui->setupUi(this);
    setMinimumSize(900, 650);
    socket = new QTcpSocket(this);
    historyDialog = new HistoryDialog(this);
    settingsWidget = new SettingsWidget_Doc();
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
    setMinimumSize(900, 650);
    if (socket) {
        socket->setParent(this);
    }
    historyDialog = new HistoryDialog(this);
    settingsWidget = new SettingsWidget_Doc();
    initConnections();
    
    if (socket->state() == QTcpSocket::ConnectedState) {
        ui->statusLabel->setText("状态：已连接");
        ui->statusLabel->setStyleSheet("color: #31ffb7; font-size: 14px; font-weight: 700;");
    }
}

void DoctorChatWidget::setUsername(const QString &username)
{
    m_username = username;
    settingsWidget->setUsername(username);
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
        QLabel {
            color: #d8f7ff;
            font-family: "Microsoft YaHei";
        }
        QLabel#titleLabel {
            color: #00e5ff;
            font: 700 24px "Microsoft YaHei";
        }
        QTextBrowser {
            background: rgba(2, 9, 20, 0.86);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
            color: #eafbff;
            padding: 18px;
            font: 14px "Microsoft YaHei";
        }
        QLineEdit {
            background: rgba(2, 9, 20, 0.86);
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
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31ffb7, stop:1 #00e5ff);
        }
        QPushButton#historyBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffcf5a, stop:1 #ff7a59);
        }
        QPushButton#settingsBtn {
            background: rgba(6, 24, 45, 0.92);
            color: #d8f7ff;
            border: 1px solid rgba(0, 229, 255, 0.75);
        }
    )");
    ui->headerWidget->setStyleSheet("background: rgba(4, 15, 31, 0.82); border: 1px solid rgba(0, 229, 255, 0.35); border-radius: 18px;");
    ui->inputWidget->setStyleSheet("background: rgba(4, 15, 31, 0.82); border: 1px solid rgba(0, 229, 255, 0.35); border-radius: 18px;");
    ui->textBrowser->setStyleSheet("QTextBrowser { background: rgba(2, 9, 20, 0.86); border: 1px solid rgba(0, 229, 255, 0.35); border-radius: 18px; color: #EAFBFF; padding: 18px; font: 14px \"Microsoft YaHei\"; }");
    ui->textBrowser->setHtml("<html><body style='font-family:Microsoft YaHei; font-size:14px; color:#EAFBFF; background-color:#020914; padding:20px;'>"
                             "<p align='center' style='margin-top:40px;'><span style='color:#00E5FF; font-size:22px; font-weight:700;'>欢迎进入医生咨询中心</span></p>"
                             "<p align='center' style='margin-top:15px;'><span style='color:#D8F7FF; font-size:14px;'>您将实时接收用户的健康咨询消息</span></p>"
                             "<p align='center' style='margin-top:20px;'><span style='color:#8BB9C8; font-size:12px;'>等待消息中...</span></p>"
                             "</body></html>");
    ui->titleLabel->setText("医生咨询中心");
    ui->historyBtn->setText("历史记录");
    ui->settingsBtn->setText("设置");
    ui->sendBtn->setText("发送回复");
    ui->lineEdit->setPlaceholderText("输入回复消息...");
    ui->historyBtn->setMinimumSize(128, 42);
    ui->settingsBtn->setMinimumSize(96, 42);
    ui->sendBtn->setMinimumSize(112, 42);
    ui->historyBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFCF5A, stop:1 #FF7A59); color: #03111D; border: 1px solid rgba(234, 251, 255, 0.75); border-radius: 16px; padding: 8px 18px; font: 700 14px \"Microsoft YaHei\"; min-width: 128px; min-height: 42px; } QPushButton:hover { background: #FFCF5A; }");
    ui->settingsBtn->setStyleSheet("QPushButton { background: rgba(6, 24, 45, 0.92); color: #D8F7FF; border: 1px solid rgba(0, 229, 255, 0.75); border-radius: 16px; padding: 8px 18px; font: 700 14px \"Microsoft YaHei\"; min-width: 96px; min-height: 42px; } QPushButton:hover { background: rgba(0, 229, 255, 0.18); }");
    ui->historyBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->settingsBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->sendBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->headerWidget->setMinimumHeight(112);

    connect(socket, &QTcpSocket::connected, this, [=]() {
        ui->statusLabel->setText("状态：已连接");
        ui->statusLabel->setStyleSheet("color: #31ffb7; font-size: 14px; font-weight: 700;");
    });
    connect(socket, &QTcpSocket::disconnected, this, [=]() {
        ui->statusLabel->setText("状态：已断开");
        ui->statusLabel->setStyleSheet("color: #ff5f7e; font-size: 14px; font-weight: 700;");
    });
    connect(socket, &QTcpSocket::readyRead, this, &DoctorChatWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [=](QAbstractSocket::SocketError) {
        ui->statusLabel->setText("状态：连接失败");
        ui->statusLabel->setStyleSheet("color: #ff5f7e; font-size: 14px; font-weight: 700;");
    });
    
    connect(ui->settingsBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_settingsBtn_clicked);
    connect(ui->historyBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_historyBtn_clicked);
    connect(ui->sendBtn, &QPushButton::clicked, this, &DoctorChatWidget::on_sendBtn_clicked);
    
    connect(settingsWidget, &SettingsWidget_Doc::logout, this, [=]() {
        this->close();
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
    QString ip = settings.value("serverIP", "127.0.0.1").toString();
    quint16 port = settings.value("serverPort", 9999).toUInt();
    
    socket->connectToHost(ip, port);
}

void DoctorChatWidget::sendMessage(const QString &message)
{
    QJsonObject obj;
    obj["type"] = "doctor_message";
    obj["message"] = message;
    obj["sender"] = "doctor";

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);

    socket->write(sendData);
    socket->flush();
}

void DoctorChatWidget::readData()
{
    m_buffer.append(socket->readAll());

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
            QString sender = obj.value("sender").toString();
            
            if (type == "login_success") {
                QString name = obj.value("name").toString();
                setWindowTitle("医生咨询中心 - " + name);
            } else if (type == "new_client") {
                QString clientName = obj.value("client_name").toString();
                appendSystemMessage(QString("用户 %1 已连接").arg(clientName));
            } else if (type == "client_message") {
                QString message = obj.value("message").toString();
                messageHistory.append(QString("%1：%2").arg(sender, message));
                appendChatMessage(sender, message, false);
            }
        }
    }
}

void DoctorChatWidget::on_sendBtn_clicked()
{
    QString message = ui->lineEdit->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    sendMessage(message);
    messageHistory.append(QString("我：%1").arg(message));

    appendChatMessage(m_username.isEmpty() ? "我" : m_username, message, true);
    ui->lineEdit->clear();
}

void DoctorChatWidget::on_historyBtn_clicked()
{
    QJsonArray historyArray;
    for (const QString &msg : messageHistory) {
        historyArray.append(msg);
    }
    historyDialog->setHistoryData(historyArray);
    historyDialog->show();
}

void DoctorChatWidget::on_settingsBtn_clicked()
{
    QSettings settings("SmartMedica", "DoctorClient");
    QString ip = settings.value("serverIP", "127.0.0.1").toString();
    quint16 port = settings.value("serverPort", 9999).toUInt();
    settingsWidget->setServerConfig(ip, port);
    
    settingsWidget->show();
    settingsWidget->raise();
    settingsWidget->activateWindow();
}

void DoctorChatWidget::appendChatMessage(const QString &sender, const QString &message, bool isSelf)
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

void DoctorChatWidget::appendSystemMessage(const QString &message)
{
    ui->textBrowser->append(QString(
        "<div style=\"margin: 10px 0; text-align: center;\">"
        "<span style=\"display:inline-block; padding: 6px 14px; border-radius: 14px; "
        "background: rgba(139,185,200,0.14); border: 1px solid rgba(139,185,200,0.28); "
        "color: #8BB9C8; font-size: 12px;\">%1</span>"
        "</div>")
        .arg(message.toHtmlEscaped()));
}
