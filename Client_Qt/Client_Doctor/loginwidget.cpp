#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , socket(nullptr)
{
    ui->setupUi(this);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::sendRequest(const QJsonObject &obj)
{
    if (!socket) return;
    
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    quint32 len = data.size();
    QByteArray sendData;
    sendData.append((char*)&len, sizeof(quint32));
    sendData.append(data);
    socket->write(sendData);
}

void LoginWidget::sendLoginRequest()
{
    QJsonObject obj;
    obj["type"] = "login";
    obj["username"] = m_username;
    obj["password"] = m_password;
    obj["role"] = "doctor";
    sendRequest(obj);
}

void LoginWidget::readData()
{
    if (!socket) return;
    
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

            if (type == "login_success") {
            } else if (type == "login_failed") {
            } else if (type == "register_success") {
                QMessageBox::information(this, "成功", "注册成功！请登录");
                ui->passwordEdit->clear();
                ui->loginBtn->setEnabled(true);
                ui->registerBtn->setEnabled(true);
            } else if (type == "register_failed") {
                QMessageBox::warning(this, "失败", "用户名已存在");
                ui->loginBtn->setEnabled(true);
                ui->registerBtn->setEnabled(true);
            }
        }
    }
}

void LoginWidget::on_loginBtn_clicked()
{
    m_username = ui->usernameEdit->text().trimmed();
    m_password = ui->passwordEdit->text();

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }

    if (!verifyLocalCredentials(m_username, m_password)) {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
        return;
    }

    ui->loginBtn->setEnabled(false);
    ui->registerBtn->setEnabled(false);
    
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, [=]() {
        sendLoginRequest();
        emit loginSuccess(m_username);
        emit loginSuccessWithSocket(m_username, socket);
    });
    connect(socket, &QTcpSocket::readyRead, this, &LoginWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [=](QAbstractSocket::SocketError) {
        emit loginSuccess(m_username);
        emit loginSuccessWithSocket(m_username, socket);
    });
    
    socket->connectToHost("127.0.0.1", 9999);
}

void LoginWidget::on_registerBtn_clicked()
{
    m_username = ui->usernameEdit->text().trimmed();
    m_password = ui->passwordEdit->text();

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }

    if (!registerLocalUser(m_username, m_password)) {
        QMessageBox::warning(this, "注册失败", "用户名已存在");
        return;
    }

    QMessageBox::information(this, "成功", "注册成功！请登录");
    ui->passwordEdit->clear();
}

void LoginWidget::on_clearBtn_clicked()
{
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
}

bool LoginWidget::verifyLocalCredentials(const QString &username, const QString &password)
{
    QFile file("doctors.json");
    if (!file.exists()) {
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    if (!obj.contains(username)) {
        return false;
    }
    
    QString storedPassword = obj[username].toString();
    return storedPassword == password;
}

bool LoginWidget::registerLocalUser(const QString &username, const QString &password)
{
    QJsonObject obj;
    QFile file("doctors.json");
    
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            obj = doc.object();
        }
    }
    
    if (obj.contains(username)) {
        return false;
    }
    
    obj[username] = password;
    
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson());
    file.close();
    
    return true;
}
