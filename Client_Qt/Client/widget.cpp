#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QTextStream>
#include <QDateTime>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    timer = new QTimer(this);
    historyTimer = new QTimer(this);
    msg = new QMessageBox(this);
    dia = new Dialog(this);
    audio = new Audio(this);
    speech = new Speech();
    msg->close();
    connect(socket,&QTcpSocket::connected,this,&Widget::connectService);
    connect(socket,&QTcpSocket::disconnected,this,&Widget::disConnectService);
    connect(socket,&QTcpSocket::errorOccurred,this,&Widget::connectError);
    connect(timer,&QTimer::timeout,this,&Widget::reconnect);
    connect(historyTimer,&QTimer::timeout,this,&Widget::onHistoryLoadTimerTick);
    connect(this,&Widget::sendInfo,dia,&Dialog::reConnectInfo);
    conFlag = 0;
    errFlag = 0;
    count = 0;
    isRecording = false;
    m_isThinking = false;
    m_isInterrupted = false;

    QString appDir = QCoreApplication::applicationDirPath();
    QString historyDir = appDir + "/chat_history";
    QDir dir;
    if(!dir.exists(historyDir))
    {
        dir.mkpath(historyDir);
    }
    m_chatHistoryFile = historyDir + "/chat.txt";

    ui->historyProgressBar->setVisible(false);
    ui->pushButton->setText("发送");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connectService()
{
    ui->pushButton_2->setText("断开连接");
    ui->textBrowser->clear();
    ui->textBrowser->append("已连接服务器");
    ui->statusLabel->setText("状态：已连接");
    connect(socket,&QTcpSocket::readyRead,this,&Widget::readData);

    ui->historyProgressBar->setValue(0);
    ui->historyProgressBar->setVisible(true);
    ui->textBrowser->append("正在读取历史聊天记录...");
    historyTimer->start(30);
}

void Widget::readData()
{
    buffer.append(socket->readAll());

    while (true) {
        if (buffer.size() < 4) {
            break;
        }

        quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<uchar*>(buffer.data()));
        qint32 totalLen = 4 + dataLen;

        if (buffer.size() < totalLen) {
            break;
        }

        QByteArray jsonData = buffer.mid(4, dataLen);
        buffer = buffer.mid(totalLen);

        QJsonParseError parseErr;
        QJsonDocument docu = QJsonDocument::fromJson(jsonData, &parseErr);
        if(parseErr.error == QJsonParseError::NoError) {
            QJsonObject obj = docu.object();
            QString type = obj.value("type").toString();
            QJsonValue value;

            if (type == "ai_response") {
                value = obj.value("data");
            } else if (type == "message") {
                value = obj.value("message");
            }

            if(value.isString()) {
                QString content = value.toString();

                if(m_isInterrupted)
                {
                    m_isThinking = false;
                    m_isInterrupted = false;
                    ui->textBrowser->append("已中断");
                    ui->pushButton->setText("发送");
                    continue;
                }

                m_isThinking = false;
                ui->pushButton->setText("发送");
                ui->textBrowser->append("茯苓：" + content);
                saveChatMessage("茯苓", content);
            }
        }
    }
}

void Widget::disConnectService()
{
    ui->pushButton_2->setText("链接服务器");
    disconnect(socket,&QTcpSocket::readyRead,this,&Widget::readData);
    conFlag = 0;
    ui->statusLabel->setText("状态：已断开");
    m_isThinking = false;
    m_isInterrupted = false;
    ui->pushButton->setText("发送");
}

void Widget::connectError(QAbstractSocket::SocketError err)
{
    if(!errFlag)
    {
        int btn = QMessageBox::warning(this,"网络错误","服务器错误:"+QString::number(err),QMessageBox::Ok|QMessageBox::Close);
        if(btn==QMessageBox::Ok)
        {
            if(conFlag)
            {
                errFlag = 1;
                timer->start(1000);
                while(!dia->exec());
            }
        }
        else
        {
            this->close();
        }
    }
    else
    {
        emit sendInfo(count);
    }
}

void Widget::reconnect()
{
    count++;
    socket->connectToHost(ui->lineEdit_2->text(),ui->lineEdit_3->text().toInt());
}

void Widget::on_pushButton_2_clicked()
{
    if(ui->pushButton_2->text()=="链接服务器")
    {
        socket->connectToHost(ui->lineEdit_2->text(),ui->lineEdit_3->text().toInt());
        conFlag = 1;
        ui->statusLabel->setText("状态：连接中...");
    }
    else
    {
        socket->disconnectFromHost();
    }
}

void Widget::on_pushButton_clicked()
{
    if(m_isThinking)
    {
        m_isInterrupted = true;
        return;
    }

    QJsonObject obj;
    obj["type"] = "message";
    QString message = ui->lineEdit->text().trimmed();
    if(message.isEmpty()) return;
    obj["data"] = message;
    ui->textBrowser->append("我：" + message);
    saveChatMessage("我", message);
    m_pendingUserMessage = message;

    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint32)jsonData.size();
    packet.append(jsonData);
    socket->write(packet);
    socket->flush();
    ui->lineEdit->clear();

    ui->textBrowser->append("思考中...");
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
    ui->textBrowser->append("开始录音...");
}

void Widget::on_voiceBtn_released()
{
    if(isRecording)
    {
        isRecording = false;
        audio->stopAudioRecord();
        ui->voiceBtn->setText("🎤");
        ui->voiceLabel->setText("按住说话，松开发送");
        ui->textBrowser->append("录音结束，正在识别...");

        QString recognizedText = speech->speechIdentify("record.wav");

        if(!recognizedText.isEmpty())
        {
            ui->textBrowser->append("识别结果：" + recognizedText);

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

            ui->textBrowser->append("我：" + recognizedText);
            saveChatMessage("我", recognizedText);

            ui->textBrowser->append("思考中...");
            ui->pushButton->setText("中断");
            m_isThinking = true;
            m_isInterrupted = false;
        }
        else
        {
            ui->textBrowser->append("识别失败，请重试");
        }
    }
}

void Widget::onHistoryLoadTimerTick()
{
    int currentValue = ui->historyProgressBar->value();
    if(currentValue < 100)
    {
        ui->historyProgressBar->setValue(currentValue + 1);
    }
    else
    {
        historyTimer->stop();
        loadChatHistory();
        ui->historyProgressBar->setVisible(false);
    }
}

void Widget::saveChatMessage(const QString &role, const QString &content)
{
    QFile file(m_chatHistoryFile);
    if(file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        out << timestamp << "|" << role << "|" << content << "\n";
        file.close();
    }
}

void Widget::loadChatHistory()
{
    QFile file(m_chatHistoryFile);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        ui->textBrowser->append("--- 历史聊天记录 ---");
        while(!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if(!line.isEmpty())
            {
                QStringList parts = line.split("|");
                if(parts.size() >= 3)
                {
                    QString role = parts[1];
                    QString content = parts[2];
                    ui->textBrowser->append(role + "：" + content);
                }
            }
        }
        ui->textBrowser->append("--- 历史记录结束 ---");
        file.close();
    }
}
