#include "medicalrecordwidget.h"
#include "ui_medicalrecordwidget.h"
#include "themehelpers.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStringConverter>
#include <QTextStream>

MedicalRecordWidget::MedicalRecordWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MedicalRecordWidget)
    , m_serverIP("127.0.0.1")
    , m_serverPort(9999)
    , m_currentMode(QStringLiteral("普通模式"))
    , socket(new QTcpSocket(this))
    , m_isAiThinking(false)
    , m_detailWidget(nullptr)
    , settingsWidget(nullptr)
    , m_fontColor("#D8F7FF")
    , m_bgColor("#07111F")
{
    ui->setupUi(this);

    connect(socket, &QTcpSocket::connected, this, &MedicalRecordWidget::onSocketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MedicalRecordWidget::onSocketDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MedicalRecordWidget::onSocketReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MedicalRecordWidget::onSocketError);

    connect(ui->settingsBtn, &QPushButton::clicked, this, &MedicalRecordWidget::onSettingsBtnClicked);
    connect(ui->aiFillBtn, &QPushButton::clicked, this, &MedicalRecordWidget::onAiFillBtnClicked);
    connect(ui->saveBtn, &QPushButton::clicked, this, &MedicalRecordWidget::onSaveBtnClicked);
    connect(ui->backBtn, &QPushButton::clicked, this, &MedicalRecordWidget::onBackBtnClicked);
    connect(ui->recordList, &QListWidget::itemClicked, this, &MedicalRecordWidget::onRecordListClicked);

    ui->dateEdit->setDate(QDate::currentDate());
    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
    refreshRecordList();
}

MedicalRecordWidget::~MedicalRecordWidget()
{
    delete ui;
}

void MedicalRecordWidget::setServerInfo(const QString &ip, int port)
{
    m_serverIP = ip;
    m_serverPort = port;

    if (socket->state() != QTcpSocket::ConnectedState && socket->state() != QTcpSocket::ConnectingState) {
        socket->connectToHost(m_serverIP, m_serverPort);
    }
}

void MedicalRecordWidget::setUsername(const QString &username)
{
    m_username = username;
    refreshRecordList();
}

void MedicalRecordWidget::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    applyModeSettings(mode);
    applyBgColor(bgColor);
    applyFontColor(fontColor);
    if (m_detailWidget) {
        m_detailWidget->applyAppearance(mode, m_bgColor, m_fontColor);
    }
}

void MedicalRecordWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    QFont font = ui->diseaseEdit->font();
    QFont labelFont = ui->dateEdit->font();
    QFont btnFont = ui->aiFillBtn->font();
    QFont titleFont = ui->titleLabel->font();

    if (mode == QStringLiteral("关怀模式")) {
        font.setPointSize(16);
        labelFont.setPointSize(15);
        btnFont.setPointSize(15);
        titleFont.setPointSize(22);

        ui->aiFillBtn->setMinimumHeight(52);
        ui->saveBtn->setMinimumHeight(52);
        ui->backBtn->setMinimumHeight(52);
        ui->settingsBtn->setMinimumHeight(52);
        ui->recordList->setSpacing(10);
    } else {
        font.setPointSize(10);
        labelFont.setPointSize(10);
        btnFont.setPointSize(10);
        titleFont.setPointSize(14);

        ui->aiFillBtn->setMinimumHeight(36);
        ui->saveBtn->setMinimumHeight(36);
        ui->backBtn->setMinimumHeight(36);
        ui->settingsBtn->setMinimumHeight(36);
        ui->recordList->setSpacing(4);
    }

    ui->diseaseEdit->setFont(font);
    ui->dateEdit->setFont(labelFont);
    ui->treatmentEdit->setFont(font);
    ui->recordList->setFont(font);
    ui->aiFillBtn->setFont(btnFont);
    ui->saveBtn->setFont(btnFont);
    ui->backBtn->setFont(btnFont);
    ui->settingsBtn->setFont(btnFont);
    ui->titleLabel->setFont(titleFont);
    ui->recordListLabel->setFont(titleFont);
    ui->diseaseLabel->setFont(labelFont);
    ui->dateLabel->setFont(labelFont);
    ui->treatmentLabel->setFont(labelFont);
}

void MedicalRecordWidget::applyFontColor(const QString &color)
{
    m_fontColor = color.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : color;
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);
    const QString inputBg = light ? "rgba(255, 255, 255, 0.94)" : "rgba(2, 9, 20, 0.86)";
    ui->diseaseEdit->setStyleSheet(QString("QLineEdit { color: %1; background-color: %2; border: 1px solid rgba(0, 229, 255, 0.38); border-radius: 14px; padding: 8px 12px; }").arg(m_fontColor, inputBg));
    ui->treatmentEdit->setStyleSheet(QString("QTextEdit { color: %1; background-color: %2; border: 1px solid rgba(0, 229, 255, 0.38); border-radius: 14px; padding: 8px 12px; }").arg(m_fontColor, inputBg));
    ui->recordList->setStyleSheet(QString("QListWidget { color: %1; background-color: %2; border: 1px solid rgba(0, 229, 255, 0.38); border-radius: 14px; padding: 8px 12px; }").arg(m_fontColor, inputBg));
    ui->statusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(light ? "#4C647A" : "#D8F7FF"));
}

void MedicalRecordWidget::applyBgColor(const QString &color)
{
    m_bgColor = ThemeHelpers::normalizeBgColor(color);
    applyFontColor(m_fontColor);
    if (!ThemeHelpers::isLightTheme(m_bgColor)) {
        setStyleSheet(R"(
            QWidget#MedicalRecordWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023);
            }
            QWidget#leftWidget {
                background: rgba(4, 15, 31, 0.76);
                border: 1px solid rgba(0, 229, 255, 0.28);
                border-radius: 18px;
            }
            QLabel#titleLabel, QLabel#recordListLabel {
                color: #00E5FF;
                font-weight: 700;
            }
        )");
    } else {
        setStyleSheet(R"(
            QWidget#MedicalRecordWidget {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF);
            }
            QWidget#leftWidget {
                background: rgba(255, 255, 255, 0.78);
                border: 1px solid rgba(15, 39, 64, 0.14);
                border-radius: 18px;
            }
            QLabel#titleLabel, QLabel#recordListLabel {
                color: #0F2740;
                font-weight: 700;
            }
        )");
    }
}

void MedicalRecordWidget::onSettingsBtnClicked()
{
    if (!settingsWidget) {
        settingsWidget = new SettingsWidget(nullptr);
        settingsWidget->setWindowModality(Qt::NonModal);
        settingsWidget->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        settingsWidget->setUsername(m_username);
        settingsWidget->setServerConfig(m_serverIP, m_serverPort, true);
        settingsWidget->setCurrentMode(m_currentMode);
        settingsWidget->setFontColor(m_fontColor);
        settingsWidget->setBgColor(m_bgColor);

        connect(settingsWidget, &SettingsWidget::logout, this, &MedicalRecordWidget::onLogoutFromSettings);
        connect(settingsWidget, &SettingsWidget::modeChanged, this, &MedicalRecordWidget::onModeChanged);
        connect(settingsWidget, &SettingsWidget::fontColorChanged, this, &MedicalRecordWidget::onFontColorChanged);
        connect(settingsWidget, &SettingsWidget::bgColorChanged, this, &MedicalRecordWidget::onBgColorChanged);
        connect(settingsWidget, &SettingsWidget::serverConfigChanged, this, [=](const QString &ip, quint16 port, bool autoConnect) {
            m_serverIP = ip;
            m_serverPort = port;
            Q_UNUSED(autoConnect);
            if (socket->state() != QTcpSocket::ConnectedState && socket->state() != QTcpSocket::ConnectingState) {
                socket->connectToHost(m_serverIP, m_serverPort);
            }
        });
        connect(settingsWidget, &SettingsWidget::destroyed, this, [=]() {
            settingsWidget = nullptr;
        });
    }
    settingsWidget->show();
    settingsWidget->raise();
    settingsWidget->activateWindow();
}

void MedicalRecordWidget::onLogoutFromSettings()
{
    if (settingsWidget) {
        settingsWidget->close();
        delete settingsWidget;
        settingsWidget = nullptr;
    }
    emit logout();
}

void MedicalRecordWidget::onFontColorChanged(const QString &color)
{
    applyFontColor(color);
    emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
}

void MedicalRecordWidget::onModeChanged(const QString &mode)
{
    applyModeSettings(mode);
    if (m_detailWidget) {
        m_detailWidget->applyAppearance(mode, m_bgColor, m_fontColor);
    }
    emit modeChanged(mode);
    emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
}

void MedicalRecordWidget::onBgColorChanged(const QString &color)
{
    applyBgColor(color);
    emit appearanceChanged(m_currentMode, m_bgColor, m_fontColor);
}

void MedicalRecordWidget::onAiFillBtnClicked()
{
    const QString diseaseName = ui->diseaseEdit->text().trimmed();
    if (diseaseName.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请先输入疾病名称。"));
        return;
    }

    if (m_isAiThinking) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("AI 正在思考中，请稍候。"));
        return;
    }

    m_isAiThinking = true;
    ui->aiFillBtn->setEnabled(false);
    ui->saveBtn->setEnabled(false);
    ui->statusLabel->setText(QStringLiteral("状态：AI 正在思考..."));

    const QString question = QStringLiteral("我得了 %1，我该怎么办？直接为我提供治疗建议").arg(diseaseName);

    if (socket->state() != QTcpSocket::ConnectedState) {
        socket->connectToHost(m_serverIP, m_serverPort);
    }

    QJsonObject obj;
    obj["type"] = "message";
    obj["data"] = question;

    const QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << (quint32)jsonData.size();
    packet.append(jsonData);

    socket->write(packet);
    socket->flush();
}

void MedicalRecordWidget::onSaveBtnClicked()
{
    const QString diseaseName = ui->diseaseEdit->text().trimmed();
    const QString diagnosisDate = ui->dateEdit->date().toString("yyyy-MM-dd");
    const QString treatment = ui->treatmentEdit->toPlainText().trimmed();

    if (diseaseName.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请输入疾病名称。"));
        return;
    }

    if (treatment.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请填写治疗建议。"));
        return;
    }

    saveRecord(diseaseName, diagnosisDate, treatment);
    QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("病例保存成功。"));

    ui->diseaseEdit->clear();
    ui->dateEdit->setDate(QDate::currentDate());
    ui->treatmentEdit->clear();

    refreshRecordList();
}

void MedicalRecordWidget::onBackBtnClicked()
{
    emit backToMenu();
}

void MedicalRecordWidget::onRecordListClicked(QListWidgetItem *item)
{
    const QString fileName = item->data(Qt::UserRole).toString();
    if (fileName.isEmpty()) {
        return;
    }

    QString diseaseName;
    QString diagnosisDate;
    QString treatment;
    loadRecordData(fileName, diseaseName, diagnosisDate, treatment);

    if (diseaseName.isEmpty()) {
        return;
    }

    if (m_detailWidget) {
        m_detailWidget->deleteLater();
        m_detailWidget = nullptr;
    }

    m_detailWidget = new RecordDetailWidget(nullptr);
    m_detailWidget->setAttribute(Qt::WA_DeleteOnClose);
    m_detailWidget->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_detailWidget->setRecordData(diseaseName, diagnosisDate, treatment);
    m_detailWidget->applyAppearance(m_currentMode, m_bgColor, m_fontColor);

    connect(m_detailWidget, &QWidget::destroyed, this, [=]() {
        m_detailWidget = nullptr;
    });

    m_detailWidget->show();
    m_detailWidget->raise();
    m_detailWidget->activateWindow();
}

void MedicalRecordWidget::loadRecordData(const QString &fileName, QString &diseaseName, QString &diagnosisDate, QString &treatment)
{
    const QString filePath = getRecordDir() + "/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    diseaseName.clear();
    diagnosisDate.clear();
    treatment.clear();

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    bool inTreatment = false;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.startsWith(QStringLiteral("疾病名称:"))) {
            diseaseName = line.mid(QStringLiteral("疾病名称:").size());
        } else if (line.startsWith(QStringLiteral("诊断日期:"))) {
            diagnosisDate = line.mid(QStringLiteral("诊断日期:").size());
        } else if (line.startsWith(QStringLiteral("治疗建议:"))) {
            inTreatment = true;
            treatment = line.mid(QStringLiteral("治疗建议:").size());
        } else if (inTreatment) {
            treatment += "\n" + line;
        }
    }
}

void MedicalRecordWidget::onSocketConnected()
{
    ui->statusLabel->setText(QStringLiteral("状态：已连接服务器"));
}

void MedicalRecordWidget::onSocketDisconnected()
{
    ui->statusLabel->setText(QStringLiteral("状态：已断开服务器"));
}

void MedicalRecordWidget::onSocketReadyRead()
{
    m_buffer.append(socket->readAll());

    while (true) {
        if (m_buffer.size() < 4) {
            break;
        }

        const quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<uchar*>(m_buffer.data()));
        const qint32 totalLen = 4 + dataLen;
        if (m_buffer.size() < totalLen) {
            break;
        }

        const QByteArray jsonData = m_buffer.mid(4, dataLen);
        m_buffer = m_buffer.mid(totalLen);

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        const QJsonObject obj = doc.object();
        const QString type = obj.value("type").toString();
        QJsonValue value;

        if (type == "ai_response") {
            value = obj.value("data");
        } else if (type == "message") {
            value = obj.value("message");
        }

        if (value.isString()) {
            ui->treatmentEdit->setPlainText(value.toString());
        }
    }

    m_isAiThinking = false;
    ui->aiFillBtn->setEnabled(true);
    ui->saveBtn->setEnabled(true);
    ui->statusLabel->setText(QStringLiteral("状态：AI 建议已生成"));
}

void MedicalRecordWidget::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_isAiThinking = false;
    ui->aiFillBtn->setEnabled(true);
    ui->saveBtn->setEnabled(true);
    ui->statusLabel->setText(QStringLiteral("状态：连接失败"));
    QMessageBox::warning(nullptr, QStringLiteral("错误"), QStringLiteral("无法连接到服务器：") + socket->errorString());
}

void MedicalRecordWidget::refreshRecordList()
{
    ui->recordList->clear();

    QDir recordDir(getRecordDir());
    if (!recordDir.exists()) {
        return;
    }

    const QStringList files = recordDir.entryList(QStringList() << "record_*.txt", QDir::Files, QDir::Time);
    for (const QString &file : files) {
        QFileInfo fi(recordDir.filePath(file));
        QString displayText = file.mid(7, 8) + " " + file.mid(16, 6) + " - ";

        QFile recordFile(fi.absoluteFilePath());
        if (recordFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&recordFile);
            in.setEncoding(QStringConverter::Utf8);
            const QString line = in.readLine();
            if (line.startsWith(QStringLiteral("疾病名称:"))) {
                displayText += line.mid(QStringLiteral("疾病名称:").size());
            }
        }

        auto *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, file);
        ui->recordList->addItem(item);
    }
}

void MedicalRecordWidget::saveRecord(const QString &diseaseName, const QString &diagnosisDate, const QString &treatment)
{
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    const QString fileName = "record_" + timestamp + ".txt";
    const QString filePath = getRecordDir() + "/" + fileName;

    QDir().mkpath(getRecordDir());

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << QStringLiteral("疾病名称:") << diseaseName << "\n";
        out << QStringLiteral("诊断日期:") << diagnosisDate << "\n";
        out << QStringLiteral("治疗建议:") << treatment << "\n";
    }
}

void MedicalRecordWidget::loadRecord(const QString &fileName)
{
    QString diseaseName;
    QString diagnosisDate;
    QString treatment;
    loadRecordData(fileName, diseaseName, diagnosisDate, treatment);

    ui->diseaseEdit->setText(diseaseName);
    ui->dateEdit->setDate(QDate::fromString(diagnosisDate, "yyyy-MM-dd"));
    ui->treatmentEdit->setPlainText(treatment);
}

QString MedicalRecordWidget::sanitizeUserName(const QString &username) const
{
    QString safeName = username.trimmed();
    if (safeName.isEmpty()) {
        safeName = QStringLiteral("anonymous");
    }
    safeName.replace(QRegularExpression(QStringLiteral(R"([\\/:*?"<>|\s]+)")), QStringLiteral("_"));
    return safeName;
}

QString MedicalRecordWidget::getRecordDir() const
{
    return QDir::homePath() + "/SmartMedica/records/" + sanitizeUserName(m_username);
}
