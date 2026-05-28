#include "doctorlistwidget.h"
#include "ui_doctorlistwidget.h"
#include "doctordialog.h"
#include <QMessageBox>
#include <QJsonDocument>

DoctorListWidget::DoctorListWidget(const QString &serverIP, int serverPort, const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorListWidget)
{
    ui->setupUi(this);
    m_serverIP = serverIP;
    m_serverPort = serverPort;
    m_username = username;

    socket = new QTcpSocket(this);
    refreshTimer = new QTimer(this);

    connect(socket, &QTcpSocket::connected, this, &DoctorListWidget::requestDoctorList);
    connect(socket, &QTcpSocket::disconnected, this, [=]() {
        ui->statusLabel->setText("状态：已断开");
        ui->statusLabel->setStyleSheet("color: red;");
    });
    connect(socket, &QTcpSocket::readyRead, this, &DoctorListWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [=](QAbstractSocket::SocketError) {
        ui->statusLabel->setText("状态：连接失败");
        ui->statusLabel->setStyleSheet("color: red;");
    });
    connect(refreshTimer, &QTimer::timeout, this, &DoctorListWidget::refreshDoctorList);
    connect(ui->doctorList, &QListWidget::itemClicked, this, &DoctorListWidget::onDoctorItemClicked);
    connect(ui->backBtn, &QPushButton::clicked, this, &DoctorListWidget::onBackBtnClicked);

    connectToServer();
    refreshTimer->start(5000);
}

DoctorListWidget::~DoctorListWidget()
{
    delete ui;
}

void DoctorListWidget::connectToServer()
{
    socket->connectToHost(m_serverIP, m_serverPort);
}

void DoctorListWidget::sendRequest(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    quint32 len = data.size();
    QByteArray header = QByteArray::fromRawData(reinterpret_cast<const char*>(&len), sizeof(quint32));
    
    QByteArray sendData;
    sendData.append((char*)&len, sizeof(quint32));
    sendData.append(data);
    
    socket->write(sendData);
}

void DoctorListWidget::requestDoctorList()
{
    ui->statusLabel->setText("状态：已连接");
    ui->statusLabel->setStyleSheet("color: green;");
    
    QJsonObject obj;
    obj["type"] = "get_doctors";
    sendRequest(obj);
}

void DoctorListWidget::readData()
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

            if (type == "doctor_list") {
                ui->doctorList->clear();
                QJsonArray doctors = obj.value("doctors").toArray();
                foreach (const QJsonValue &val, doctors) {
                    QJsonObject doctor = val.toObject();
                    QString name = doctor.value("name").toString();
                    bool online = doctor.value("online").toBool();
                    
                    QString itemText = QString("%1 %2").arg(name).arg(online ? "[在线]" : "[离线]");
                    QListWidgetItem *item = new QListWidgetItem(itemText);
                    item->setData(Qt::UserRole, doctor.value("id").toInt());
                    item->setData(Qt::UserRole + 1, online);
                    
                    QColor color = online ? QColor(Qt::green) : QColor(Qt::gray);
                    item->setForeground(color);
                    ui->doctorList->addItem(item);
                }
            }
        }
    }
}

void DoctorListWidget::onDoctorItemClicked(QListWidgetItem *item)
{
    int doctorId = item->data(Qt::UserRole).toInt();
    bool online = item->data(Qt::UserRole + 1).toBool();
    
    if (!online) {
        QMessageBox::warning(this, "提示", "该医生当前不在线");
        return;
    }

    QJsonObject obj;
    obj["type"] = "connect_doctor";
    obj["doctor_id"] = doctorId;
    sendRequest(obj);

    DoctorDialog *dialog = new DoctorDialog(socket, m_username, item->text().split(" ")[0], this);
    dialog->show();
}

void DoctorListWidget::refreshDoctorList()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        requestDoctorList();
    }
}

void DoctorListWidget::onBackBtnClicked()
{
    socket->disconnectFromHost();
    emit backToMenu();
}
