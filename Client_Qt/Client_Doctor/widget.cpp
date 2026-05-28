#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    timer = new QTimer(this);
    msg = new QMessageBox(this);
    dia = new Dialog(this);
    audio = new Audio(this);
    msg->close();
    connect(socket,&QTcpSocket::connected,this,&Widget::connectService);
    connect(socket,&QTcpSocket::disconnected,this,&Widget::disConnectService);
    connect(socket,&QTcpSocket::errorOccurred,this,&Widget::connectError);
    connect(timer,&QTimer::timeout,this,&Widget::reconnect);
    connect(this,&Widget::sendInfo,dia,&Dialog::reConnectInfo);
    conFlag = 0;
    errFlag = 0;
    count = 0;
    isRecording = false;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connectService()
{
    ui->pushButton_2->setText("断开连接");
    ui->textBrowser->append("已连接服务器");
    ui->statusLabel->setText("状态：已连接");
    connect(socket,&QTcpSocket::readyRead,this,&Widget::readData);
}

void Widget::readData()
{
    QJsonParseError err;
    QJsonValue value;
    int len = socket->read(data,4);
    quint32 strLen = qFromBigEndian<quint32>(reinterpret_cast<uchar*>(data));
    ui->textBrowser->append("len:"+QString::number(len)+",datalen:"+QString::number(strLen));
    QByteArray arrayData = socket->read(strLen);
    QJsonDocument docu = QJsonDocument::fromJson(arrayData,&err);
    if(err.error == QJsonParseError::NoError)
    {
        QJsonObject obj = docu.object();
        value = obj.value("message");
    }
    if(value.isString())
        ui->textBrowser->append("茯苓："+value.toString());
}

void Widget::disConnectService()
{
    ui->pushButton_2->setText("链接服务器");
    disconnect(socket,&QTcpSocket::readyRead,this,&Widget::readData);
    conFlag = 0;
    ui->statusLabel->setText("状态：已断开");
}

void Widget::connectError(QAbstractSocket::SocketError err)
{
    if(!errFlag)
    {
        int btn = QMessageBox::warning(nullptr,"网络错误","服务器错误:"+QString::number(err),QMessageBox::Ok|QMessageBox::Close);
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
    QJsonObject obj;
    obj["type"] = "message";
    QString message = ui->lineEdit->text().trimmed();
    if(message.isEmpty()) return;
    obj["message"] = message;
    ui->textBrowser->append("我：" + message);
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    ui->lineEdit->clear();
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
        ui->textBrowser->append("录音结束");
    }
}
