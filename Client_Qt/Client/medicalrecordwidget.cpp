
#include "medicalrecordwidget.h"
#include "ui_medicalrecordwidget.h"
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QListWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>
#include <QFileInfo>
#include <QStringConverter>

MedicalRecordWidget::MedicalRecordWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MedicalRecordWidget),
    m_serverIP("127.0.0.1"),
    m_serverPort(9999),
    m_currentMode("普通模式"),
    m_isAiThinking(false),
    m_detailWidget(nullptr),
    settingsWidget(nullptr),
    m_fontColor("#000000"),
    m_bgColor("#ffffff")
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
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
}

void MedicalRecordWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;
    
    QFont font = ui->diseaseEdit->font();
    QFont labelFont = ui->dateEdit->font();
    QFont btnFont = ui->aiFillBtn->font();
    QFont titleFont = ui->titleLabel->font();
    
    if (mode == "关怀模式") {
        font.setPointSize(font.pointSize() * 1.5);
        labelFont.setPointSize(labelFont.pointSize() * 1.5);
        btnFont.setPointSize(btnFont.pointSize() * 1.5);
        titleFont.setPointSize(titleFont.pointSize() * 1.5);
        
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
    } else {
        font.setPointSize(10);
        labelFont.setPointSize(10);
        btnFont.setPointSize(10);
        titleFont.setPointSize(14);
        
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
}

void MedicalRecordWidget::applyFontColor(const QString &color)
{
    m_fontColor = color;
    ui->diseaseEdit->setStyleSheet(QString("QLineEdit { color: %1; background-color: %2; }").arg(color).arg(m_bgColor));
    ui->treatmentEdit->setStyleSheet(QString("QTextEdit { color: %1; background-color: %2; }").arg(color).arg(m_bgColor));
    ui->recordList->setStyleSheet(QString("QListWidget { color: %1; background-color: %2; }").arg(color).arg(m_bgColor));
}

void MedicalRecordWidget::applyBgColor(const QString &color)
{
    m_bgColor = color;
    ui->diseaseEdit->setStyleSheet(QString("QLineEdit { color: %1; background-color: %2; }").arg(m_fontColor).arg(color));
    ui->treatmentEdit->setStyleSheet(QString("QTextEdit { color: %1; background-color: %2; }").arg(m_fontColor).arg(color));
    ui->recordList->setStyleSheet(QString("QListWidget { color: %1; background-color: %2; }").arg(m_fontColor).arg(color));
    this->setStyleSheet(QString("QWidget { background-color: %1; }").arg(color));
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
}

void MedicalRecordWidget::onModeChanged(const QString &mode)
{
    applyModeSettings(mode);
}

void MedicalRecordWidget::onBgColorChanged(const QString &color)
{
    applyBgColor(color);
}

void MedicalRecordWidget::onAiFillBtnClicked()
{
    QString diseaseName = ui->diseaseEdit->text().trimmed();
    if (diseaseName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先输入疾病名称");
        return;
    }

    if (m_isAiThinking) {
        QMessageBox::warning(this, "提示", "AI正在思考中，请等待");
        return;
    }

    m_isAiThinking = true;
    ui->aiFillBtn->setEnabled(false);
    ui->saveBtn->setEnabled(false);
    ui->statusLabel->setText("状态：AI正在思考...");

    QString question = QString("我得了%1，我该怎么办？直接为我提供治疗建议").arg(diseaseName);

    if (socket->state() != QTcpSocket::ConnectedState) {
        socket->connectToHost(m_serverIP, m_serverPort);
    }

    QJsonObject obj;
    obj["type"] = "message";
    obj["data"] = question;

    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
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
    QString diseaseName = ui->diseaseEdit->text().trimmed();
    QString diagnosisDate = ui->dateEdit->date().toString("yyyy-MM-dd");
    QString treatment = ui->treatmentEdit->toPlainText().trimmed();

    if (diseaseName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入疾病名称");
        return;
    }

    if (treatment.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写治疗建议");
        return;
    }

    saveRecord(diseaseName, diagnosisDate, treatment);
    QMessageBox::information(this, "提示", "病例保存成功");

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
    QString fileName = item->data(Qt::UserRole).toString();
    if (!fileName.isEmpty()) {
        QString diseaseName, diagnosisDate, treatment;
        loadRecordData(fileName, diseaseName, diagnosisDate, treatment);

        if (!diseaseName.isEmpty()) {
            if (m_detailWidget) {
                m_detailWidget->deleteLater();
                m_detailWidget = nullptr;
            }
            m_detailWidget = new RecordDetailWidget(nullptr);
            m_detailWidget->setAttribute(Qt::WA_DeleteOnClose);
            m_detailWidget->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
            m_detailWidget->setRecordData(diseaseName, diagnosisDate, treatment);
            m_detailWidget->applyModeSettings(m_currentMode);

            connect(m_detailWidget, &QWidget::destroyed, this, [=]() {
                m_detailWidget = nullptr;
            });

            m_detailWidget->show();
            m_detailWidget->raise();
            m_detailWidget->activateWindow();
        }
    }
}

void MedicalRecordWidget::loadRecordData(const QString &fileName, QString &diseaseName, QString &diagnosisDate, QString &treatment)
{
    QString filePath = getRecordDir() + "/" + fileName;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        diseaseName.clear();
        diagnosisDate.clear();
        treatment.clear();

        QStringList allLines;
        QTextStream in(&file);
        while (!in.atEnd()) {
            allLines << in.readLine();
        }
        file.close();

        bool inTreatment = false;

        for (int i = 0; i < allLines.size(); i++) {
            QString line = allLines[i];
            if (line.startsWith("疾病名称:")) {
                diseaseName = line.mid(5);
            } else if (line.startsWith("诊断日期:")) {
                diagnosisDate = line.mid(5);
            } else if (line.startsWith("治疗建议:")) {
                inTreatment = true;
                treatment = line.mid(5);
            } else if (inTreatment && !line.isEmpty()) {
                treatment += "\n" + line;
            }
        }
    }
}

void MedicalRecordWidget::onSocketConnected()
{
    ui->statusLabel->setText("状态：已连接服务器");
}

void MedicalRecordWidget::onSocketDisconnected()
{
    ui->statusLabel->setText("状态：已断开服务器");
}

void MedicalRecordWidget::onSocketReadyRead()
{
    m_buffer.append(socket->readAll());

    while (true) {
        if (m_buffer.size() < 4) break;

        quint32 dataLen = qFromBigEndian<quint32>(reinterpret_cast<uchar*>(m_buffer.data()));
        qint32 totalLen = 4 + dataLen;

        if (m_buffer.size() < totalLen) break;

        QByteArray jsonData = m_buffer.mid(4, dataLen);
        m_buffer = m_buffer.mid(totalLen);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            QString type = obj.value("type").toString();
            QJsonValue value;

            if (type == "ai_response")
                value = obj.value("data");
            else if (type == "message")
                value = obj.value("message");

            if (value.isString()) {
                QString content = value.toString();
                ui->treatmentEdit->setPlainText(content);
            }
        }
    }

    m_isAiThinking = false;
    ui->aiFillBtn->setEnabled(true);
    ui->saveBtn->setEnabled(true);
    ui->statusLabel->setText("状态：AI建议已生成");
}

void MedicalRecordWidget::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_isAiThinking = false;
    ui->aiFillBtn->setEnabled(true);
    ui->saveBtn->setEnabled(true);
    ui->statusLabel->setText("状态：连接失败");
    QMessageBox::warning(this, "错误", "无法连接到服务器：" + socket->errorString());
}

void MedicalRecordWidget::refreshRecordList()
{
    ui->recordList->clear();

    QDir recordDir(getRecordDir());
    if (!recordDir.exists()) {
        return;
    }

    QStringList filters;
    filters << "record_*.txt";
    QStringList files = recordDir.entryList(filters, QDir::Files, QDir::Time);

    for (const QString &file : files) {
        QFileInfo fi(recordDir.filePath(file));
        QString displayText = file.mid(7, 8) + " " + file.mid(15, 4) + " - ";

        QFile recordFile(fi.absoluteFilePath());
        if (recordFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&recordFile);
            QString line = in.readLine();
            if (line.startsWith("疾病名称:")) {
                displayText += line.mid(5);
            }
            recordFile.close();
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, file);
        ui->recordList->addItem(item);
    }
}

void MedicalRecordWidget::saveRecord(const QString &diseaseName, const QString &diagnosisDate, const QString &treatment)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName = "record_" + timestamp + ".txt";
    QString filePath = getRecordDir() + "/" + fileName;

    QDir().mkpath(getRecordDir());

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << "疾病名称:" << diseaseName << "\n";
        out << "诊断日期:" << diagnosisDate << "\n";
        out << "治疗建议:" << treatment << "\n";
        file.close();
    }
}

void MedicalRecordWidget::loadRecord(const QString &fileName)
{
    QString filePath = getRecordDir() + "/" + fileName;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->diseaseEdit->clear();
        ui->dateEdit->clear();
        ui->treatmentEdit->clear();
        
        QStringList allLines;
        QTextStream in(&file);
        while (!in.atEnd()) {
            allLines << in.readLine();
        }
        file.close();
        
        QString diseaseName, diagnosisDate, treatment;
        bool inTreatment = false;
        
        for (int i = 0; i < allLines.size(); i++) {
            QString line = allLines[i];
            if (line.startsWith("疾病名称:")) {
                diseaseName = line.mid(5);
            } else if (line.startsWith("诊断日期:")) {
                diagnosisDate = line.mid(5);
            } else if (line.startsWith("治疗建议:")) {
                inTreatment = true;
                treatment = line.mid(5);
            } else if (inTreatment && !line.isEmpty()) {
                treatment += "\n" + line;
            }
        }
        
        if (!diseaseName.isEmpty()) {
            ui->diseaseEdit->setText(diseaseName);
        }
        if (!diagnosisDate.isEmpty()) {
            ui->dateEdit->setDate(QDate::fromString(diagnosisDate, "yyyy-MM-dd"));
        }
        if (!treatment.isEmpty()) {
            ui->treatmentEdit->setPlainText(treatment);
        }
    }
}

QString MedicalRecordWidget::getRecordDir() const
{
    return QDir::homePath() + "/SmartMedica/records";
}
