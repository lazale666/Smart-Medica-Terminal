#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    recvBuffer.clear();

    // 构造函数中一次性绑定所有信号，避免重复绑定
    connect(socket, &QTcpSocket::connected, this, &Widget::connectService);
    connect(socket, &QTcpSocket::disconnected, this, &Widget::disConnectService);
    connect(socket, &QTcpSocket::errorOccurred, this, &Widget::connectError);
    connect(socket, &QTcpSocket::readyRead, this, &Widget::readData);
}

Widget::~Widget()
{
    // 安全关闭连接，释放资源
    if(socket->isOpen()){
        socket->disconnectFromHost();
        socket->waitForDisconnected(3000); // 等待3秒确保断开
    }
    delete ui;
}

void Widget::connectService()
{
    ui->pushButton_2->setText("断开连接");
    ui->textBrowser->append("服务器连接成功！");
}

void Widget::disConnectService()
{
    ui->pushButton_2->setText("连接服务器");
    recvBuffer.clear(); // 断开连接时清空缓冲区
    ui->textBrowser->append("已断开服务器连接");
}

void Widget::connectError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    ui->textBrowser->append("️连接错误：" + socket->errorString());
    QMessageBox::warning(this, "连接错误", socket->errorString());
}

void Widget::readData()
{
    // 1. 把新收到的数据追加到缓冲区
    recvBuffer.append(socket->readAll());

    // 2. 循环处理缓冲区，直到数据不足
    while (recvBuffer.size() >= 4) {
        // 读取4字节长度头
        quint32 dataLen = qFromBigEndian<quint32>(recvBuffer.constData());
        // 判断缓冲区是否有完整的JSON数据
        if (recvBuffer.size() < 4 + dataLen) {
            // 数据不完整，等待后续数据到达
            break;
        }

        // 3. 提取完整的JSON数据
        QByteArray jsonData = recvBuffer.mid(4, dataLen);
        // 4. 从缓冲区移除已处理的数据
        recvBuffer.remove(0, 4 + dataLen);

        // 5. 解析JSON
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
        if (err.error != QJsonParseError::NoError) {
            ui->textBrowser->append("JSON解析失败：" + err.errorString());
            continue;
        }

        QJsonObject obj = doc.object();
        if (obj.contains("message")) {
            QString msg = obj["message"].toString();
            ui->textBrowser->append("收到消息：" + msg);
        }
    }
}

void Widget::on_pushButton_2_clicked()
{
    if(ui->pushButton_2->text() == "连接服务器"){
        QString ip = ui->lineEdit_2->text();
        quint16 port = ui->lineEdit_3->text().toUShort();
        socket->connectToHost(ip, port);
    }else{
        socket->disconnectFromHost();
    }
}

void Widget::on_pushButton_clicked()
{
    if(!socket->isOpen()){
        QMessageBox::warning(this, "提示", "请先连接服务器！");
        return;
    }

    // 1. 构造JSON数据
    QJsonObject obj;
    obj["type"] = "message";
    obj["message"] = ui->lineEdit->text();
    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    // 2. 构造4字节大端长度头
    quint32 len = jsonData.size();
    QByteArray header;
    header.resize(4);
    qToBigEndian(len, reinterpret_cast<uchar*>(header.data()));

    // 3. 先发送长度头，再发送JSON数据
    socket->write(header);
    socket->write(jsonData);
    socket->flush();

    ui->textBrowser->append("已发送消息：" + ui->lineEdit->text());
    ui->lineEdit->clear();
}