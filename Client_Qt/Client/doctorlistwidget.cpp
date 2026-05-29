#include "doctorlistwidget.h"
#include "ui_doctorlistwidget.h"
#include "doctordialog.h"
#include "themehelpers.h"

#include <QJsonArray>
#include <QtEndian>
#include <algorithm>

DoctorListWidget::DoctorListWidget(const QString &serverIP, int serverPort, const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoctorListWidget)
    , m_serverIP(serverIP)
    , m_serverPort(serverPort)
    , m_username(username)
    , m_currentMode("普通模式")
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
    , refreshTimer(nullptr)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &DoctorListWidget::requestDoctorList);
    connect(socket, &QTcpSocket::disconnected, this, [=]() {
        m_isConnecting = false;
        if (!m_hasActiveSession) {
            setDoctorSelectionEnabled(true);
        }
        ui->statusLabel->setText(QStringLiteral("状态：已断开"));
        ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusErrorColor(m_bgColor)));
    });
    connect(socket, &QTcpSocket::readyRead, this, &DoctorListWidget::readData);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, [=](QAbstractSocket::SocketError) {
        m_isConnecting = false;
        if (!m_hasActiveSession) {
            setDoctorSelectionEnabled(true);
        }
        ui->statusLabel->setText(QStringLiteral("状态：连接失败"));
        ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusErrorColor(m_bgColor)));
    });
    connect(ui->backBtn, &QPushButton::clicked, this, &DoctorListWidget::onBackBtnClicked);
    connect(ui->doctorList, &QListWidget::itemClicked, this, &DoctorListWidget::onDoctorItemClicked);

    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
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

void DoctorListWidget::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(bgColor);
    m_fontColor = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : fontColor;
    applyModeSettings(mode);

    if (ThemeHelpers::isLightTheme(m_bgColor)) {
        setStyleSheet(R"(
            QWidget#DoctorListWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF);
            }
            QListWidget {
                background: rgba(255, 255, 255, 0.92);
                border: 1px solid rgba(15, 39, 64, 0.14);
                border-radius: 18px;
                color: #0F2740;
                padding: 12px;
                font: 14px "Microsoft YaHei";
            }
        )");
    } else {
        setStyleSheet(R"(
            QWidget#DoctorListWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111f, stop:0.55 #071b2f, stop:1 #0b1023);
            }
            QListWidget {
                background: rgba(2, 9, 20, 0.82);
                border: 1px solid rgba(0, 229, 255, 0.35);
                border-radius: 18px;
                color: #eafbff;
                padding: 12px;
                font: 14px "Microsoft YaHei";
            }
        )");
    }

    ui->titleLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::titleColor(m_bgColor)));
    ui->hintLabel->setStyleSheet(QString("color: %1;").arg(m_fontColor));
    renderDoctorList();
}

void DoctorListWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    QFont titleFont = ui->titleLabel->font();
    QFont labelFont = ui->statusLabel->font();
    QFont listFont = ui->doctorList->font();
    QFont buttonFont = ui->backBtn->font();

    if (mode == QStringLiteral("关怀模式")) {
        titleFont.setPointSize(28);
        labelFont.setPointSize(15);
        listFont.setPointSize(16);
        buttonFont.setPointSize(15);
        ui->doctorList->setSpacing(10);
        ui->backBtn->setMinimumHeight(48);
        resize(860, 660);
    } else {
        titleFont.setPointSize(20);
        labelFont.setPointSize(12);
        listFont.setPointSize(14);
        buttonFont.setPointSize(12);
        ui->doctorList->setSpacing(4);
        ui->backBtn->setMinimumHeight(34);
        resize(640, 520);
    }

    ui->titleLabel->setFont(titleFont);
    ui->statusLabel->setFont(labelFont);
    ui->hintLabel->setFont(labelFont);
    ui->doctorList->setFont(listFont);
    ui->backBtn->setFont(buttonFont);
}

void DoctorListWidget::sendRequest(const QJsonObject &obj)
{
    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    const quint32 len = qToBigEndian<quint32>(static_cast<quint32>(data.size()));

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
        const quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(m_buffer.constData()));
        if (m_buffer.size() < static_cast<int>(4 + dataLen)) {
            break;
        }

        const QByteArray jsonData = m_buffer.mid(4, dataLen);
        m_buffer = m_buffer.mid(4 + dataLen);

        const QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isObject()) {
            continue;
        }

        const QJsonObject obj = doc.object();
        const QString type = obj.value("type").toString();

        if (type == "doctor_list") {
            m_doctors.clear();
            const QJsonArray doctorsArray = obj.value("doctors").toArray();
            for (const QJsonValue &value : doctorsArray) {
                const QJsonObject doctorObj = value.toObject();
                DoctorListEntry entry;
                entry.id = doctorObj.value("id").toInteger();
                entry.name = doctorObj.value("name").toString();
                entry.online = doctorObj.value("online").toBool();
                entry.activeClients = doctorObj.value("active_clients").toInt();
                m_doctors.append(entry);
            }

            std::sort(m_doctors.begin(), m_doctors.end(), [](const DoctorListEntry &left, const DoctorListEntry &right) {
                if (left.online != right.online) {
                    return left.online && !right.online;
                }
                return QString::localeAwareCompare(left.name, right.name) < 0;
            });

            renderDoctorList();
            if (!m_isConnecting && !m_hasActiveSession) {
                ui->statusLabel->setText(m_doctors.isEmpty() ? QStringLiteral("状态：当前无在线医师") : QStringLiteral("状态：请选择在线医师"));
            }
            ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;")
                                               .arg(m_doctors.isEmpty() ? ThemeHelpers::statusWarnColor(m_bgColor)
                                                                        : ThemeHelpers::statusOkColor(m_bgColor)));
        } else if (type == "connection_success") {
            m_isConnecting = false;
            m_hasActiveSession = true;
            setDoctorSelectionEnabled(false);
            ui->statusLabel->setText(QStringLiteral("状态：已连接医师"));
            ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusOkColor(m_bgColor)));

            disconnect(socket, &QTcpSocket::readyRead, this, &DoctorListWidget::readData);
            const QString doctorName = obj.value("doctor_name").toString(QStringLiteral("医生"));
            const QString sessionId = obj.value("session_id").toString();
            m_activeDialog = new DoctorDialog(socket, m_username, doctorName, sessionId, this);
            m_activeDialog->applyAppearance(m_currentMode, m_bgColor, m_fontColor);
            m_activeDialog->setAttribute(Qt::WA_DeleteOnClose);
            m_activeDialog->show();

            connect(m_activeDialog, &QDialog::finished, this, [=]() {
                m_hasActiveSession = false;
                m_isConnecting = false;
                m_selectedDoctorId = -1;
                m_activeDialog = nullptr;
                socket->disconnectFromHost();
                emit backToMenu();
            });
        } else if (type == "waiting_for_doctor") {
            m_isConnecting = false;
            setDoctorSelectionEnabled(true);
            ui->statusLabel->setText(QStringLiteral("状态：暂无医师在线，请稍候..."));
            ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusWarnColor(m_bgColor)));
        } else if (type == "connection_failed") {
            m_isConnecting = false;
            m_hasActiveSession = false;
            m_selectedDoctorId = -1;
            setDoctorSelectionEnabled(true);
            const QString message = obj.value("message").toString();
            ui->statusLabel->setText(message.isEmpty()
                                         ? QStringLiteral("状态：连接失败，已刷新列表")
                                         : QStringLiteral("状态：%1").arg(message));
            ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusErrorColor(m_bgColor)));
            requestOnlineDoctors();
        }
    }
}

void DoctorListWidget::onBackBtnClicked()
{
    closeActiveWindowsAndReturn();
}

void DoctorListWidget::onDoctorItemClicked(QListWidgetItem *item)
{
    if (m_isConnecting || m_hasActiveSession) {
        return;
    }

    const qint64 doctorId = item->data(Qt::UserRole).toLongLong();
    if (doctorId <= 0) {
        return;
    }

    m_isConnecting = true;
    m_selectedDoctorId = doctorId;
    setDoctorSelectionEnabled(false);
    ui->statusLabel->setText(QStringLiteral("状态：正在连接所选医师..."));
    ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusWarnColor(m_bgColor)));

    QJsonObject obj;
    obj["type"] = "connect_doctor";
    obj["doctor_id"] = doctorId;
    obj["username"] = m_username;
    sendRequest(obj);
}

void DoctorListWidget::renderDoctorList()
{
    ui->doctorList->clear();
    const QColor onlineColor(ThemeHelpers::statusOkColor(m_bgColor));
    const QColor offlineColor(ThemeHelpers::isLightTheme(m_bgColor) ? "#6D8192" : "#8BB9C8");

    for (const DoctorListEntry &doctor : m_doctors) {
        const QString statusText = doctor.online ? QStringLiteral("[在线]") : QStringLiteral("[离线]");
        const QString loadText = doctor.online
            ? QStringLiteral("当前接诊 %1 人").arg(doctor.activeClients)
            : QStringLiteral("当前不可接诊");
        QListWidgetItem *item = new QListWidgetItem(QString("%1    %2\n%3").arg(doctor.name, statusText, loadText), ui->doctorList);
        item->setData(Qt::UserRole, QString::number(doctor.id));
        item->setForeground(doctor.online ? onlineColor : offlineColor);
    }
}

void DoctorListWidget::requestOnlineDoctors()
{
    ui->statusLabel->setText(QStringLiteral("状态：正在获取在线医师列表..."));
    ui->statusLabel->setStyleSheet(QString("color: %1; font-weight: 700;").arg(ThemeHelpers::statusWarnColor(m_bgColor)));

    QJsonObject obj;
    obj["type"] = "get_doctors";
    sendRequest(obj);
}

void DoctorListWidget::setDoctorSelectionEnabled(bool enabled)
{
    ui->doctorList->setEnabled(enabled);
}

void DoctorListWidget::closeActiveWindowsAndReturn()
{
    m_isConnecting = false;
    m_hasActiveSession = false;
    m_selectedDoctorId = -1;
    setDoctorSelectionEnabled(true);

    if (m_activeDialog) {
        DoctorDialog *dialog = m_activeDialog;
        m_activeDialog = nullptr;
        dialog->close();
    }

    if (socket) {
        socket->disconnectFromHost();
    }

    close();
    emit backToMenu();
}
