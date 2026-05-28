#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <QMessageBox>
#include <QColorDialog>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QTextStream>
#include <QStringConverter>
#include <QListWidgetItem>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    m_settings = new QSettings("SmartMedica", "Client", this);
    m_currentColor = "#000000";
    m_currentBgColor = "#ffffff";

    connect(ui->navUserInfoBtn, &QPushButton::clicked, this, &SettingsWidget::onNavUserInfoClicked);
    connect(ui->navModeBtn, &QPushButton::clicked, this, &SettingsWidget::onNavModeClicked);
    connect(ui->navCacheBtn, &QPushButton::clicked, this, &SettingsWidget::onNavCacheClicked);
    connect(ui->navServerBtn, &QPushButton::clicked, this, &SettingsWidget::onNavServerClicked);

    connect(ui->logoutBtn, &QPushButton::clicked, this, &SettingsWidget::onLogoutBtnClicked);
    connect(ui->modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWidget::onModeChanged);
    connect(ui->colorBtn, &QPushButton::clicked, this, &SettingsWidget::onColorBtnClicked);
    connect(ui->bgColorBtn, &QPushButton::clicked, this, &SettingsWidget::onBgColorBtnClicked);
    connect(ui->saveServerBtn, &QPushButton::clicked, this, &SettingsWidget::onSaveServerBtnClicked);
    connect(ui->saveUserInfoBtn, &QPushButton::clicked, this, &SettingsWidget::onSaveUserInfoBtnClicked);

    connect(ui->volumeSlider, &QSlider::valueChanged, this, &SettingsWidget::onVolumeSliderChanged);
    connect(ui->rateSlider, &QSlider::valueChanged, this, &SettingsWidget::onRateSliderChanged);

    connect(ui->selectAllChatBtn, &QPushButton::clicked, this, &SettingsWidget::onSelectAllChatClicked);
    connect(ui->deleteChatBtn, &QPushButton::clicked, this, &SettingsWidget::onDeleteChatClicked);
    connect(ui->selectAllMedicalBtn, &QPushButton::clicked, this, &SettingsWidget::onSelectAllMedicalClicked);
    connect(ui->deleteMedicalBtn, &QPushButton::clicked, this, &SettingsWidget::onDeleteMedicalClicked);

    ui->navUserInfoBtn->setChecked(true);
    switchToPage(0);

    loadSettings();
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::setUsername(const QString &username)
{
    m_currentUsername = username;
    ui->usernameLabel->setText(username);
}

void SettingsWidget::setServerConfig(const QString &ip, quint16 port, bool autoConnect)
{
    ui->ipEdit->setText(ip);
    ui->portEdit->setText(QString::number(port));
    ui->autoConnectCheck->setChecked(autoConnect);
}

void SettingsWidget::setCurrentMode(const QString &mode)
{
    bool showColor = !(mode == "极简模式" || mode == "关怀模式");
    ui->colorLabel->setVisible(showColor);
    ui->colorBtn->setVisible(showColor);
    ui->colorPreview->setVisible(showColor);
    
    if (mode == "极简模式") {
        ui->modeCombo->setCurrentIndex(1);
    } else if (mode == "关怀模式") {
        ui->modeCombo->setCurrentIndex(2);
    } else {
        ui->modeCombo->setCurrentIndex(0);
    }
}

void SettingsWidget::setFontColor(const QString &color)
{
    m_currentColor = color;
    // 更新合并后的大预览
    ui->colorPreview->setStyleSheet(QString("background-color: %1; color: %2;")
                                    .arg(m_currentBgColor)
                                    .arg(color));
}

void SettingsWidget::setBgColor(const QString &color)
{
    m_currentBgColor = color;
    // 更新合并后的大预览
    ui->colorPreview->setStyleSheet(QString("background-color: %1; color: %2;")
                                    .arg(color)
                                    .arg(m_currentColor));
}

void SettingsWidget::switchToPage(int pageIndex)
{
    ui->pageStack->setCurrentIndex(pageIndex);

    ui->navUserInfoBtn->setChecked(pageIndex == 0);
    ui->navModeBtn->setChecked(pageIndex == 1);
    ui->navCacheBtn->setChecked(pageIndex == 2);
    ui->navServerBtn->setChecked(pageIndex == 3);

    if (pageIndex == 2) {
        loadChatRecords();
        loadMedicalRecords();
    }
}

void SettingsWidget::loadSettings()
{
    QString mode = m_settings->value("mode", "普通模式").toString();
    QString color = m_settings->value("fontColor", "#000000").toString();
    QString bgColor = m_settings->value("bgColor", "#ffffff").toString();
    QString ip = m_settings->value("serverIP", "127.0.0.1").toString();
    quint16 port = m_settings->value("serverPort", 9999).toUInt();
    bool autoConnect = m_settings->value("autoConnect", true).toBool();

    QString gender = m_settings->value("gender", "保密").toString();
    int age = m_settings->value("age", 0).toInt();

    int volume = m_settings->value("speechVolume", 100).toInt();
    int rate = m_settings->value("speechRate", 50).toInt();

    setCurrentMode(mode);
    setBgColor(bgColor);
    setFontColor(color);
    setServerConfig(ip, port, autoConnect);

    int genderIndex = 2; // 默认保密
    if (gender == "男") genderIndex = 0;
    else if (gender == "女") genderIndex = 1;
    ui->genderCombo->setCurrentIndex(genderIndex);
    ui->ageSpinBox->setValue(age);

    ui->volumeSlider->setValue(volume);
    ui->rateSlider->setValue(rate);
}

void SettingsWidget::saveSettings()
{
    m_settings->setValue("mode", ui->modeCombo->currentText());
    m_settings->setValue("fontColor", m_currentColor);
    m_settings->setValue("bgColor", m_currentBgColor);
    m_settings->setValue("serverIP", ui->ipEdit->text());
    m_settings->setValue("serverPort", ui->portEdit->text().toUInt());
    m_settings->setValue("autoConnect", ui->autoConnectCheck->isChecked());
    m_settings->sync();
}

void SettingsWidget::onSaveUserInfoBtnClicked()
{
    QString gender = ui->genderCombo->currentText();
    int age = ui->ageSpinBox->value();

    m_settings->setValue("gender", gender);
    m_settings->setValue("age", age);
    m_settings->sync();

    QMessageBox::information(this, "提示", "用户信息已保存！");
}

void SettingsWidget::loadChatRecords()
{
    ui->chatRecordList->clear();
    QString appDir = QCoreApplication::applicationDirPath();
    QString historyDir = appDir + "/chat_history";
    QDir dir(historyDir);

    if (!dir.exists()) {
        return;
    }

    QStringList filters;
    filters << "chat_*.txt";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);

    for (const QFileInfo &fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            QString firstLine = in.readLine();
            file.close();

            QString displayText = fileInfo.fileName();
            if (!firstLine.isEmpty()) {
                QStringList parts = firstLine.split("|");
                if (parts.size() >= 3 && parts[1] == "我") {
                    displayText = parts[2].left(30);
                    if (parts[2].length() > 30) displayText += "...";
                }
            }

            QListWidgetItem *item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, fileInfo.fileName());
            ui->chatRecordList->addItem(item);
        }
    }
}

void SettingsWidget::loadMedicalRecords()
{
    ui->medicalRecordList->clear();
    QString recordDir = QDir::homePath() + "/SmartMedica/records";
    QDir dir(recordDir);

    if (!dir.exists()) {
        return;
    }

    QStringList filters;
    filters << "record_*.txt";
    QStringList files = dir.entryList(filters, QDir::Files, QDir::Time);

    for (const QString &fileName : files) {
        QString filePath = dir.filePath(fileName);
        QFile file(filePath);
        
        QString displayText = fileName;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString line = in.readLine();
            if (line.startsWith("疾病名称:")) {
                displayText = fileName.mid(7, 8) + " " + line.mid(5);
            }
            file.close();
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, fileName);
        ui->medicalRecordList->addItem(item);
    }
}

void SettingsWidget::onNavUserInfoClicked()
{
    switchToPage(0);
}

void SettingsWidget::onNavModeClicked()
{
    switchToPage(1);
}

void SettingsWidget::onNavCacheClicked()
{
    switchToPage(2);
}

void SettingsWidget::onNavServerClicked()
{
    switchToPage(3);
}

void SettingsWidget::onSaveServerBtnClicked()
{
    saveSettings();
    QString ip = ui->ipEdit->text();
    quint16 port = ui->portEdit->text().toUInt();
    bool autoConnect = ui->autoConnectCheck->isChecked();
    emit serverConfigChanged(ip, port, autoConnect);
    QMessageBox::information(this, "提示", "服务器配置已保存");
}

void SettingsWidget::onCloseBtnClicked()
{
    saveSettings();
    emit closeSettings();
}

void SettingsWidget::onLogoutBtnClicked()
{
    int ret = QMessageBox::question(this, "确认退出", "确定要退出登录吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        saveSettings();
        emit logout();
    }
}

void SettingsWidget::onModeChanged(int index)
{
    QString mode = ui->modeCombo->itemText(index);
    if (mode == "极简模式" || mode == "关怀模式") {
        ui->colorLabel->setVisible(false);
        ui->colorBtn->setVisible(false);
        ui->bgColorLabel->setVisible(false);
        ui->bgColorBtn->setVisible(false);
        ui->colorPreview->setVisible(false);
        m_currentColor = "#000000";
        m_currentBgColor = "#ffffff";
        saveSettings();
        emit fontColorChanged(m_currentColor);
        emit bgColorChanged(m_currentBgColor);
    } else {
        ui->colorLabel->setVisible(true);
        ui->colorBtn->setVisible(true);
        ui->bgColorLabel->setVisible(true);
        ui->bgColorBtn->setVisible(true);
        ui->colorPreview->setVisible(true);
        ui->colorPreview->setStyleSheet(QString("background-color: %1; color: %2;")
                                        .arg(m_currentBgColor)
                                        .arg(m_currentColor));
    }

    saveSettings();
    emit modeChanged(mode);
}

void SettingsWidget::onColorBtnClicked()
{
    QColor color = QColorDialog::getColor(QColor(m_currentColor), this, "选择字体颜色");
    if (color.isValid()) {
        m_currentColor = color.name();
        setFontColor(m_currentColor);

        saveSettings();
        emit fontColorChanged(m_currentColor);
    }
}

void SettingsWidget::onBgColorBtnClicked()
{
    QColor color = QColorDialog::getColor(QColor(m_currentBgColor), this, "选择背景颜色");
    if (color.isValid()) {
        m_currentBgColor = color.name();
        setBgColor(m_currentBgColor);

        saveSettings();
        emit bgColorChanged(m_currentBgColor);
    }
}

void SettingsWidget::onSelectAllChatClicked()
{
    for (int i = 0; i < ui->chatRecordList->count(); ++i) {
        ui->chatRecordList->item(i)->setSelected(true);
    }
}

void SettingsWidget::onDeleteChatClicked()
{
    QList<QListWidgetItem*> selectedItems = ui->chatRecordList->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的聊天记录");
        return;
    }

    int ret = QMessageBox::warning(this, "确认删除", 
        QString("确定要删除选中的 %1 条聊天记录吗？此操作不可恢复！").arg(selectedItems.size()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString historyDir = appDir + "/chat_history";
        QDir dir(historyDir);

        int deletedCount = 0;
        for (QListWidgetItem *item : selectedItems) {
            QString fileName = item->data(Qt::UserRole).toString();
            if (dir.remove(fileName)) {
                deletedCount++;
                delete item;
            }
        }

        QMessageBox::information(this, "删除成功", QString("成功删除 %1 条聊天记录").arg(deletedCount));
        emit cacheCleared();
    }
}

void SettingsWidget::onSelectAllMedicalClicked()
{
    for (int i = 0; i < ui->medicalRecordList->count(); ++i) {
        ui->medicalRecordList->item(i)->setSelected(true);
    }
}

void SettingsWidget::onDeleteMedicalClicked()
{
    QList<QListWidgetItem*> selectedItems = ui->medicalRecordList->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的病例记录");
        return;
    }

    int ret = QMessageBox::warning(this, "确认删除", 
        QString("确定要删除选中的 %1 条病例记录吗？此操作不可恢复！").arg(selectedItems.size()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QString recordDir = QDir::homePath() + "/SmartMedica/records";
        QDir dir(recordDir);

        int deletedCount = 0;
        for (QListWidgetItem *item : selectedItems) {
            QString fileName = item->data(Qt::UserRole).toString();
            if (dir.remove(fileName)) {
                deletedCount++;
                delete item;
            }
        }

        QMessageBox::information(this, "删除成功", QString("成功删除 %1 条病例记录").arg(deletedCount));
    }
}

void SettingsWidget::onVolumeSliderChanged(int value)
{
    ui->volumeValueLabel->setText(QString::number(value) + "%");

    double volume = value / 100.0;
    m_settings->setValue("speechVolume", value);

    emit speechSettingsChanged(volume, ui->rateSlider->value() / 50.0 - 1.0);
}

void SettingsWidget::onRateSliderChanged(int value)
{
    QString rateText;
    if (value < 30) {
        rateText = "慢";
    } else if (value < 70) {
        rateText = "中";
    } else {
        rateText = "快";
    }
    ui->rateValueLabel->setText(rateText);

    double rate = value / 50.0 - 1.0;
    m_settings->setValue("speechRate", value);

    emit speechSettingsChanged(ui->volumeSlider->value() / 100.0, rate);
}
