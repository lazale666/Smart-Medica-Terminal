#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "themehelpers.h"

#include <QColorDialog>
#include <QDir>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTextStream>
#include <QStringConverter>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    setMinimumSize(720, 520);

    m_settings = new QSettings("SmartMedica", "Client", this);
    m_currentColor = "#D8F7FF";
    m_currentBgColor = "#07111F";

    connect(ui->navUserInfoBtn, &QPushButton::clicked, this, &SettingsWidget::onNavUserInfoClicked);
    connect(ui->navModeBtn, &QPushButton::clicked, this, &SettingsWidget::onNavModeClicked);
    connect(ui->navCacheBtn, &QPushButton::clicked, this, &SettingsWidget::onNavCacheClicked);
    connect(ui->navServerBtn, &QPushButton::clicked, this, &SettingsWidget::onNavServerClicked);

    connect(ui->logoutBtn, &QPushButton::clicked, this, &SettingsWidget::onLogoutBtnClicked);
    connect(ui->modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWidget::onModeChanged);
    connect(ui->colorBtn, &QPushButton::clicked, this, &SettingsWidget::onColorBtnClicked);
    connect(ui->bgStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsWidget::onBgStyleChanged);
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
    if (mode == QStringLiteral("极简模式")) {
        ui->modeCombo->setCurrentIndex(1);
    } else if (mode == QStringLiteral("关怀模式")) {
        ui->modeCombo->setCurrentIndex(2);
    } else {
        ui->modeCombo->setCurrentIndex(0);
    }
}

void SettingsWidget::setFontColor(const QString &color)
{
    m_currentColor = color.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_currentBgColor) : color;
    updateTheme();
}

void SettingsWidget::setBgColor(const QString &color)
{
    m_currentBgColor = ThemeHelpers::normalizeBgColor(color);
    ui->bgStyleCombo->setCurrentIndex(ThemeHelpers::isLightTheme(m_currentBgColor) ? 1 : 0);
    updateTheme();
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
    const QString mode = m_settings->value("mode", "普通模式").toString();
    const QString bgColor = ThemeHelpers::normalizeBgColor(m_settings->value("bgColor", "#07111F").toString());
    const QString color = m_settings->value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
    const QString ip = m_settings->value("serverIP", "127.0.0.1").toString();
    const quint16 port = m_settings->value("serverPort", 9999).toUInt();
    const bool autoConnect = m_settings->value("autoConnect", true).toBool();
    const QString gender = m_settings->value("gender", "保密").toString();
    const int age = m_settings->value("age", 0).toInt();
    const int volume = m_settings->value("speechVolume", 100).toInt();
    const int rate = m_settings->value("speechRate", 50).toInt();

    setCurrentMode(mode);
    setBgColor(bgColor);
    setFontColor(color);
    setServerConfig(ip, port, autoConnect);

    int genderIndex = 2;
    if (gender == QStringLiteral("男")) {
        genderIndex = 0;
    } else if (gender == QStringLiteral("女")) {
        genderIndex = 1;
    }

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

void SettingsWidget::updateTheme()
{
    const bool light = ThemeHelpers::isLightTheme(m_currentBgColor);
    const QString mainText = light ? "#0F2740" : "#D8F7FF";
    const QString panelBg = light ? "rgba(255, 255, 255, 0.78)" : "transparent";
    const QString panelBorder = light ? "1px solid rgba(15, 39, 64, 0.12)" : "none";
    const QString titleColor = ThemeHelpers::titleColor(m_currentBgColor);
    const QString listBg = light ? "rgba(255, 255, 255, 0.92)" : "rgba(2, 9, 20, 0.82)";
    const QString comboPopupBg = light ? "#FFFFFF" : "#081523";
    const QString deleteButtonBg = light ? "#E86A73" : "#ff6b6b";

    setStyleSheet(QString(R"(
        QWidget#SettingsWidget {
            background: %1;
        }
        QStackedWidget, QWidget#userInfoPage, QWidget#modePage, QWidget#cachePage, QWidget#serverPage {
            background: transparent;
        }
        QLabel {
            color: %2;
            font-family: "Microsoft YaHei";
        }
        QLabel#usernameLabel, QLabel#cacheLabel, QLabel#serverLabel {
            color: %3;
            font-weight: 700;
        }
        QPushButton#navUserInfoBtn, QPushButton#navModeBtn, QPushButton#navCacheBtn, QPushButton#navServerBtn {
            font-weight: 700;
        }
        QGroupBox {
            background: %4;
            border: %5;
            border-radius: 16px;
            margin-top: 12px;
            color: %2;
            font-weight: 700;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 8px;
            color: %3;
        }
        QListWidget {
            background: %6;
            color: %2;
            border: 1px solid rgba(0, 229, 255, 0.22);
            border-radius: 14px;
            padding: 8px;
        }
        QComboBox QAbstractItemView {
            background: %7;
            color: %2;
            selection-background-color: #C7F4FF;
            selection-color: #03111D;
        }
        QLabel#colorPreview {
            background-color: %8;
            color: %9;
            border: 1px solid rgba(0, 229, 255, 0.22);
            border-radius: 14px;
        }
        QPushButton#deleteChatBtn, QPushButton#deleteMedicalBtn {
            background-color: %10;
            color: white;
            border: none;
        }
    )")
        .arg(light
                 ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF)"
                 : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023)",
             mainText,
             titleColor,
             panelBg,
             panelBorder,
             listBg,
             comboPopupBg,
             m_currentBgColor,
             m_currentColor,
             deleteButtonBg));

    ui->colorPreview->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 14px;").arg(m_currentBgColor, m_currentColor));
}

void SettingsWidget::onSaveUserInfoBtnClicked()
{
    m_settings->setValue("gender", ui->genderCombo->currentText());
    m_settings->setValue("age", ui->ageSpinBox->value());
    m_settings->sync();
    QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("用户信息已保存。"));
}

void SettingsWidget::loadChatRecords()
{
    ui->chatRecordList->clear();
    const QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history";
    QDir dir(historyDir);
    if (!dir.exists()) {
        return;
    }

    dir.setNameFilters(QStringList() << "chat_*.txt");
    const QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
    for (const QFileInfo &fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        QString displayText = fileInfo.fileName();
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            const QString firstLine = in.readLine();
            file.close();
            if (!firstLine.isEmpty()) {
                const QStringList parts = firstLine.split("|");
                if (parts.size() >= 3 && parts[1] == QStringLiteral("我")) {
                    displayText = parts[2].left(30);
                    if (parts[2].length() > 30) {
                        displayText += "...";
                    }
                }
            }
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, fileInfo.fileName());
        ui->chatRecordList->addItem(item);
    }
}

void SettingsWidget::loadMedicalRecords()
{
    ui->medicalRecordList->clear();
    const QString recordDir = QDir::homePath() + "/SmartMedica/records";
    QDir dir(recordDir);
    if (!dir.exists()) {
        return;
    }

    const QStringList files = dir.entryList(QStringList() << "record_*.txt", QDir::Files, QDir::Time);
    for (const QString &fileName : files) {
        QListWidgetItem *item = new QListWidgetItem(fileName);
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
    emit serverConfigChanged(ui->ipEdit->text(), ui->portEdit->text().toUInt(), ui->autoConnectCheck->isChecked());
    QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("服务器配置已保存。"));
}

void SettingsWidget::onCloseBtnClicked()
{
    saveSettings();
    emit closeSettings();
}

void SettingsWidget::onLogoutBtnClicked()
{
    if (QMessageBox::question(nullptr, QStringLiteral("确认退出"), QStringLiteral("确定要退出登录吗？"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        saveSettings();
        emit logout();
    }
}

void SettingsWidget::onModeChanged(int index)
{
    Q_UNUSED(index);
    saveSettings();
    emit modeChanged(ui->modeCombo->currentText());
}

void SettingsWidget::onColorBtnClicked()
{
    QColor color = QColorDialog::getColor(QColor(m_currentColor), this, QStringLiteral("选择字体颜色"));
    if (!color.isValid()) {
        return;
    }

    m_currentColor = color.name();
    updateTheme();
    saveSettings();
    emit fontColorChanged(m_currentColor);
}

void SettingsWidget::onBgStyleChanged(int index)
{
    m_currentBgColor = (index == 0) ? "#07111F" : "#F5FBFF";
    m_currentColor = ThemeHelpers::defaultFontColorForBg(m_currentBgColor);
    updateTheme();
    saveSettings();
    emit bgColorChanged(m_currentBgColor);
    emit fontColorChanged(m_currentColor);
}

void SettingsWidget::onSelectAllChatClicked()
{
    for (int i = 0; i < ui->chatRecordList->count(); ++i) {
        ui->chatRecordList->item(i)->setSelected(true);
    }
}

void SettingsWidget::onDeleteChatClicked()
{
    const QList<QListWidgetItem*> selectedItems = ui->chatRecordList->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请先选择要删除的聊天记录。"));
        return;
    }

    const QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history";
    QDir dir(historyDir);
    int deletedCount = 0;
    for (QListWidgetItem *item : selectedItems) {
        const QString fileName = item->data(Qt::UserRole).toString();
        if (dir.remove(fileName)) {
            ++deletedCount;
            delete item;
        }
    }

    QMessageBox::information(nullptr, QStringLiteral("删除成功"), QStringLiteral("成功删除 %1 条聊天记录。").arg(deletedCount));
    emit cacheCleared();
}

void SettingsWidget::onSelectAllMedicalClicked()
{
    for (int i = 0; i < ui->medicalRecordList->count(); ++i) {
        ui->medicalRecordList->item(i)->setSelected(true);
    }
}

void SettingsWidget::onDeleteMedicalClicked()
{
    const QList<QListWidgetItem*> selectedItems = ui->medicalRecordList->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("提示"), QStringLiteral("请先选择要删除的病例记录。"));
        return;
    }

    const QString recordDir = QDir::homePath() + "/SmartMedica/records";
    QDir dir(recordDir);
    int deletedCount = 0;
    for (QListWidgetItem *item : selectedItems) {
        const QString fileName = item->data(Qt::UserRole).toString();
        if (dir.remove(fileName)) {
            ++deletedCount;
            delete item;
        }
    }

    QMessageBox::information(nullptr, QStringLiteral("删除成功"), QStringLiteral("成功删除 %1 条病例记录。").arg(deletedCount));
}

void SettingsWidget::onVolumeSliderChanged(int value)
{
    ui->volumeValueLabel->setText(QString::number(value) + "%");
    m_settings->setValue("speechVolume", value);
    emit speechSettingsChanged(value / 100.0, ui->rateSlider->value() / 50.0 - 1.0);
}

void SettingsWidget::onRateSliderChanged(int value)
{
    QString rateText = QStringLiteral("中");
    if (value < 30) {
        rateText = QStringLiteral("慢");
    } else if (value >= 70) {
        rateText = QStringLiteral("快");
    }
    ui->rateValueLabel->setText(rateText);

    m_settings->setValue("speechRate", value);
    emit speechSettingsChanged(ui->volumeSlider->value() / 100.0, value / 50.0 - 1.0);
}
