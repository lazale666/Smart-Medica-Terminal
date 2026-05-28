#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QFileInfo>
#include <QPalette>
#include <QPixmap>
#include <QBrush>
#include <QtEndian>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , socket(nullptr)
    , m_bgPath("")
{
    ui->setupUi(this);

    QString bgPath = QCoreApplication::applicationDirPath() + "/photo/background.png";
    if (!QFileInfo::exists(bgPath)) {
        bgPath = QCoreApplication::applicationDirPath() + "/../Client/photo/background.png";
    }
    if (!QFileInfo::exists(bgPath)) {
        bgPath = "D:/All Program/agant_example/Smart-Medica-Terminal/Client_Qt/Client/photo/background.png";
    }
    m_bgPath = bgPath;

    setFixedSize(1017, 398);
    setStyleSheet(R"(
        QLabel#titleLabel {
            color: #EAFBFF;
            font: 700 30px "Microsoft YaHei";
            letter-spacing: 1px;
        }
        QLineEdit {
            background: rgba(4, 15, 31, 0.78);
            border: 1px solid rgba(0, 229, 255, 0.75);
            border-radius: 16px;
            color: #EAFBFF;
            padding: 12px 18px;
            font: 14px "Microsoft YaHei";
            min-height: 24px;
        }
        QLineEdit:focus {
            border: 2px solid #00E5FF;
            background: rgba(6, 24, 45, 0.88);
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);
            border: 1px solid rgba(234, 251, 255, 0.75);
            border-radius: 16px;
            color: #03111D;
            padding: 10px 24px;
            font: 700 14px "Microsoft YaHei";
            min-height: 28px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31FFB7, stop:1 #00E5FF);
        }
        QPushButton:disabled {
            background: rgba(92, 112, 130, 0.75);
            color: rgba(234, 251, 255, 0.55);
        }
    )");
    ui->titleLabel->setStyleSheet("color: #EAFBFF; font: 700 30px \"Microsoft YaHei\";");
    updateBackground();
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
    quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);
    socket->write(sendData);
    socket->flush();
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

            if (type == "login_success") {
                disconnect(socket, &QTcpSocket::readyRead, this, &LoginWidget::readData);
                ui->loginBtn->setEnabled(true);
                ui->registerBtn->setEnabled(true);
                emit loginSuccess(m_username);
                emit loginSuccessWithSocket(m_username, socket);
            } else if (type == "login_failed") {
                QMessageBox::warning(nullptr, "登录失败", "服务器拒绝登录，请检查医生账号或重新注册。");
                ui->loginBtn->setEnabled(true);
                ui->registerBtn->setEnabled(true);
            } else if (type == "register_success") {
                QMessageBox::information(nullptr, "成功", "注册成功！请登录");
                ui->passwordEdit->clear();
                ui->loginBtn->setEnabled(true);
                ui->registerBtn->setEnabled(true);
            } else if (type == "register_failed") {
                QMessageBox::warning(nullptr, "失败", "用户名已存在");
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
        QMessageBox::warning(nullptr, "提示", "请输入用户名和密码");
        return;
    }

    if (!verifyLocalCredentials(m_username, m_password)) {
        QMessageBox::warning(nullptr, "登录失败", "用户名或密码错误");
        return;
    }

    ui->loginBtn->setEnabled(false);
    ui->registerBtn->setEnabled(false);

    if (socket) {
        socket->deleteLater();
        socket = nullptr;
    }
    m_buffer.clear();

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, [=]() {
        sendLoginRequest();
    });
    connect(socket, &QTcpSocket::readyRead, this, &LoginWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [=](QAbstractSocket::SocketError) {
        QMessageBox::warning(nullptr, "连接失败", "无法连接到服务器");
        ui->loginBtn->setEnabled(true);
        ui->registerBtn->setEnabled(true);
    });
    
    socket->connectToHost("127.0.0.1", 9999);
}

void LoginWidget::on_registerBtn_clicked()
{
    m_username = ui->usernameEdit->text().trimmed();
    m_password = ui->passwordEdit->text();

    if (m_username.isEmpty() || m_password.isEmpty()) {
        QMessageBox::warning(nullptr, "提示", "请输入用户名和密码");
        return;
    }

    if (!registerLocalUser(m_username, m_password)) {
        QMessageBox::warning(nullptr, "注册失败", "用户名已存在");
        return;
    }

    QMessageBox::information(nullptr, "成功", "注册成功！请登录");
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

void LoginWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBackground();
}

void LoginWidget::updateBackground()
{
    if (m_bgPath.isEmpty()) return;

    QPixmap background(m_bgPath);
    if (background.isNull()) return;

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(background.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    setPalette(palette);
    setAutoFillBackground(true);
}
