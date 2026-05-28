#include "doctorlistwidget.h"
#include "ui_doctorlistwidget.h"
#include "doctordialog.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QDebug>
#include <QJsonArray>
#include <QtEndian>
#include <algorithm>

DoctorListWidget::DoctorListWidget(const QString &serverIP, int serverPort, const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorListWidget)
{
    ui->setupUi(this);
    m_serverIP = serverIP;
    m_serverPort = serverPort;
    m_username = username;
    setMinimumSize(640, 520);
    setStyleSheet(R"(
        QWidget#DoctorListWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111f, stop:0.55 #071b2f, stop:1 #0b1023);
        }
        QLabel {
            color: #d8f7ff;
            font-family: "Microsoft YaHei";
        }
        QLabel#titleLabel {
            color: #00e5ff;
            font: 700 24px "Microsoft YaHei";
        }
        QListWidget {
            background: rgba(2, 9, 20, 0.82);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
            color: #eafbff;
            padding: 12px;
            font: 14px "Microsoft YaHei";
        }
        QPushButton {
            background: rgba(6, 24, 45, 0.92);
            color: #d8f7ff;
            border: 1px solid rgba(0, 229, 255, 0.65);
            border-radius: 16px;
            padding: 8px 18px;
            font: 700 14px "Microsoft YaHei";
            min-height: 34px;
        }
        QPushButton:hover {
            background: rgba(0, 229, 255, 0.18);
        }
    )");
    ui->titleLabel->setText("名医对话");
    ui->hintLabel->setText("正在为您连接在线医生...");
    ui->backBtn->setText("返回菜单");
    ui->statusLabel->setText("状态：未连接");

    socket = new QTcpSocket(this);
    
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
    connect(ui->backBtn, &QPushButton::clicked, this, &DoctorListWidget::onBackBtnClicked);
    connect(ui->doctorList, &QListWidget::itemClicked, this, &DoctorListWidget::onDoctorItemClicked);

    connectToServer();
}

DoctorListWidget::~DoctorListWidget()
{
    delete ui;
}

void DoctorListWidget::connectToServer()
{
    m_buffer.clear();
    socket->connectToHost(m_serverIP, m_serverPort);
}

void DoctorListWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    QFont titleFont = ui->titleLabel->font();
    QFont labelFont = ui->statusLabel->font();
    QFont listFont = ui->doctorList->font();
    QFont buttonFont = ui->backBtn->font();

    if (mode == "关怀模式") {
        titleFont.setPointSize(28);
        labelFont.setPointSize(15);
        listFont.setPointSize(16);
        buttonFont.setPointSize(15);

        ui->titleLabel->setFont(titleFont);
        ui->statusLabel->setFont(labelFont);
        ui->hintLabel->setFont(labelFont);
        ui->doctorList->setFont(listFont);
        ui->backBtn->setFont(buttonFont);
        ui->doctorList->setSpacing(10);
        ui->backBtn->setMinimumHeight(48);
        resize(860, 660);
    } else {
        titleFont.setPointSize(20);
        labelFont.setPointSize(12);
        listFont.setPointSize(14);
        buttonFont.setPointSize(12);

        ui->titleLabel->setFont(titleFont);
        ui->statusLabel->setFont(labelFont);
        ui->hintLabel->setFont(labelFont);
        ui->doctorList->setFont(listFont);
        ui->backBtn->setFont(buttonFont);
        ui->doctorList->setSpacing(4);
        ui->backBtn->setMinimumHeight(34);
        resize(640, 520);
    }
}

void DoctorListWidget::sendRequest(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));
    
    QByteArray sendData;
    sendData.append(reinterpret_cast<const char*>(&len), sizeof(quint32));
    sendData.append(data);
    
    socket->write(sendData);
    socket->flush();
}

void DoctorListWidget::requestDoctorList()
{
    requestOnlineDoctors();
}

void DoctorListWidget::readData()
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

            if (type == "doctor_list") {
                m_doctors.clear();
                const QJsonArray doctorsArray = obj.value("doctors").toArray();
                for (const QJsonValue &value : doctorsArray) {
                    const QJsonObject doctorObj = value.toObject();
                    DoctorListEntry entry;
                    entry.id = doctorObj.value("id").toInteger();
                    entry.name = doctorObj.value("name").toString();
                    entry.online = doctorObj.value("online").toBool();
                    m_doctors.append(entry);
                }

                std::sort(m_doctors.begin(), m_doctors.end(), [](const DoctorListEntry &left, const DoctorListEntry &right) {
                    if (left.online != right.online) {
                        return left.online && !right.online;
                    }
                    return QString::localeAwareCompare(left.name, right.name) < 0;
                });

                renderDoctorList();
                ui->statusLabel->setText(m_doctors.isEmpty() ? "状态：当前无在线医师" : "状态：请选择在线医师");
                ui->statusLabel->setStyleSheet(m_doctors.isEmpty() ? "color: #ffcf5a; font-weight: 700;" : "color: #31ffb7; font-weight: 700;");
            } else if (type == "connection_success") {
                ui->statusLabel->setText("已连接医生");
                ui->statusLabel->setStyleSheet("color: #31ffb7; font-weight: 700;");
                
                disconnect(socket, &QTcpSocket::readyRead, this, &DoctorListWidget::readData);
                const QString doctorName = obj.value("doctor_name").toString("医生");
                DoctorDialog *dialog = new DoctorDialog(socket, m_username, doctorName, this);
                dialog->applyModeSettings(m_currentMode);
                dialog->setAttribute(Qt::WA_DeleteOnClose);
                dialog->show();
                
                connect(dialog, &QDialog::finished, this, [=]() {
                    socket->disconnectFromHost();
                    emit backToMenu();
                });
            } else if (type == "waiting_for_doctor") {
                ui->statusLabel->setText("状态：暂无医生在线，请稍候...");
                ui->statusLabel->setStyleSheet("color: #ffcf5a; font-weight: 700;");
            } else if (type == "connection_failed") {
                ui->statusLabel->setText("状态：医师连接失败，已刷新列表");
                ui->statusLabel->setStyleSheet("color: #ff5f7e; font-weight: 700;");
                requestOnlineDoctors();
            } else if (type == "new_client") {
                QString doctorName = obj.value("doctor_name").toString();
                ui->statusLabel->setText("已连接医生：" + doctorName);
                ui->statusLabel->setStyleSheet("color: green;");
            }
        }
    }
}

void DoctorListWidget::onBackBtnClicked()
{
    socket->disconnectFromHost();
    emit backToMenu();
}

void DoctorListWidget::onDoctorItemClicked(QListWidgetItem *item)
{
    const qint64 doctorId = item->data(Qt::UserRole).toLongLong();
    if (doctorId <= 0) {
        return;
    }

    ui->statusLabel->setText("状态：正在连接所选医师...");
    ui->statusLabel->setStyleSheet("color: #ffcf5a; font-weight: 700;");

    QJsonObject obj;
    obj["type"] = "connect_doctor";
    obj["doctor_id"] = QString::number(doctorId).toLongLong();
    obj["username"] = m_username;
    sendRequest(obj);
}

void DoctorListWidget::renderDoctorList()
{
    ui->doctorList->clear();
    for (const DoctorListEntry &doctor : m_doctors) {
        const QString display = QString("%1    [在线]").arg(doctor.name);
        QListWidgetItem *item = new QListWidgetItem(display, ui->doctorList);
        item->setData(Qt::UserRole, QString::number(doctor.id));
        item->setForeground(QColor(doctor.online ? "#31FFB7" : "#8BB9C8"));
    }
}

void DoctorListWidget::requestOnlineDoctors()
{
    ui->statusLabel->setText("状态：正在获取在线医师列表...");
    ui->statusLabel->setStyleSheet("color: #ffcf5a; font-weight: 700;");

    QJsonObject obj;
    obj["type"] = "get_doctors";
    sendRequest(obj);
}
