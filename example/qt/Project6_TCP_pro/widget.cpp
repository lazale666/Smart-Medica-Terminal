#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    timer = new QTimer(this);
    dia = new Dialog(this);

    conFlag = 0;
    errflag = 0;
    count = 0;
    recvBuffer.clear();

    // 重连间隔 1 秒
    timer->setInterval(1000);

    // Qt6 信号槽
    connect(socket, &QTcpSocket::connected, this, &Widget::connectService);
    connect(socket, &QTcpSocket::disconnected, this, &Widget::disConnectService);
    connect(socket, &QTcpSocket::errorOccurred, this, &Widget::connectError);
    connect(timer, &QTimer::timeout, this, &Widget::reconnect);
    connect(this, &Widget::sendInfo, dia, &Dialog::reConnectInfo);
}

Widget::~Widget()
{
    delete ui;
}

// 连接成功
void Widget::connectService()
{
    ui->pushButton_2->setText("断开连接");
    count = 0;
    errflag = 0;
    timer->stop();
    dia->hide();
    recvBuffer.clear();

    connect(socket, &QTcpSocket::readyRead, this, &Widget::readData);
    QMessageBox::information(this, "成功", "服务器已连接！");
}

// 断开连接 → 自动启动重连
void Widget::disConnectService()
{
    ui->pushButton_2->setText("连接服务器");
    disconnect(socket, &QTcpSocket::readyRead, this, &Widget::readData);

    // 手动断开不重连
    if(!timer->isActive() && errflag == 1){
        count = 0;
        timer->start();
        dia->show();
    }
}

// 连接错误
void Widget::connectError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err);
    qDebug() << "错误：" << socket->errorString();

    // 第一次错误才弹提示
    if(!errflag){
        int btn = QMessageBox::warning(this,"网络异常","连接断开，是否自动重连？",
                                       QMessageBox::Ok | QMessageBox::Close);
        if(btn == QMessageBox::Ok){
            errflag = 1;
            count = 0;
            timer->start();
            dia->show();
        }else{
            close();
        }
    }
}

// 接收数据
void Widget::readData()
{
    QByteArray data = socket->readAll();
    ui->textBrowser->append("接收：" + data);
}

// 自动重连（带弹窗 + 最多10次）
void Widget::reconnect()
{
    // 最多重连 10 次
    if(count >= 10){
        timer->stop();
        dia->hide();
        errflag = 0;
        QMessageBox::critical(this,"失败","重连10次失败，停止重连！");
        return;
    }

    count++;
    emit sendInfo(count); // 弹窗更新次数

    socket->abort();
    socket->connectToHost(ui->lineEdit_2->text(), ui->lineEdit_3->text().toUShort());
}

// 连接按钮
void Widget::on_pushButton_2_clicked()
{
    if(ui->pushButton_2->text() == "连接服务器"){
        errflag = 1; // 允许断线重连
        socket->abort();
        socket->connectToHost(ui->lineEdit_2->text(), ui->lineEdit_3->text().toUShort());
    }else{
        timer->stop();
        dia->hide();
        errflag = 0;
        count = 0;
        socket->disconnectFromHost();
    }
}

// 发送消息（修复可发送）
void Widget::on_pushButton_clicked()
{
    if(socket->state() != QTcpSocket::ConnectedState){
        QMessageBox::warning(this,"提示","未连接！");
        return;
    }

    QString msg = ui->lineEdit->text();
    if(msg.isEmpty()) return;

    socket->write(msg.toUtf8());
    socket->flush();

    ui->textBrowser->append("发送：" + msg);
    ui->lineEdit->clear();
}