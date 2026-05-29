#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "resourcepaths.h"
#include "themehelpers.h"

#include <QColorDialog>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGroupBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QSizePolicy>
#include <QTextStream>
#include <QStringConverter>
#include <QVBoxLayout>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    setMinimumSize(720, 520);
    resize(900, 680);

    m_settings = new QSettings("SmartMedica", "Client", this);
    m_currentColor = "#D8F7FF";
    m_currentBgColor = "#07111F";

    configureStaticTexts();
    setupAboutPage();
    rebuildModePageLayout();

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

    if (layout()) {
        layout()->activate();
    }
    adjustSize();
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::configureStaticTexts()
{
    setWindowTitle(QStringLiteral("设置"));
    ui->navUserInfoBtn->setText(QStringLiteral("用户信息"));
    ui->navModeBtn->setText(QStringLiteral("模式切换"));
    ui->navCacheBtn->setText(QStringLiteral("缓存管理"));
    ui->navServerBtn->setText(QStringLiteral("服务器配置"));
    ui->userIconLabel->setText(QStringLiteral("用户"));
    ui->usernameLabel->setText(QStringLiteral("用户名"));
    ui->genderLabel->setText(QStringLiteral("性别："));
    ui->ageLabel->setText(QStringLiteral("年龄："));
    ui->genderCombo->setItemText(0, QStringLiteral("男"));
    ui->genderCombo->setItemText(1, QStringLiteral("女"));
    ui->genderCombo->setItemText(2, QStringLiteral("保密"));
    ui->saveUserInfoBtn->setText(QStringLiteral("保存信息"));
    ui->logoutBtn->setText(QStringLiteral("退出登录"));

    ui->modeLabel->setText(QStringLiteral("选择模式："));
    ui->modeCombo->setItemText(0, QStringLiteral("普通模式"));
    ui->modeCombo->setItemText(1, QStringLiteral("极简模式"));
    ui->modeCombo->setItemText(2, QStringLiteral("关怀模式"));
    ui->colorLabel->setText(QStringLiteral("字体颜色："));
    ui->colorBtn->setText(QStringLiteral("选择颜色"));
    ui->bgColorLabel->setText(QStringLiteral("背景样式："));
    ui->bgStyleCombo->setItemText(0, QStringLiteral("深色样式"));
    ui->bgStyleCombo->setItemText(1, QStringLiteral("浅色样式"));
    ui->bgStyleHintLabel->setText(QStringLiteral("浅色样式会切换为明亮背景与深色文字，深色样式保持当前科技蓝深色界面。"));
    ui->colorPreview->setText(QStringLiteral("预览效果"));
    ui->volumeLabel->setText(QStringLiteral("朗读音量："));
    ui->rateLabel->setText(QStringLiteral("朗读语速："));
    ui->volumeValueLabel->setText(QStringLiteral("100%"));
    ui->rateValueLabel->setText(QStringLiteral("中"));
    ui->modeDescLabel->setText(QStringLiteral("极简模式：隐藏调色功能，界面简洁"));
    ui->modeDescLabel2->setText(QStringLiteral("普通模式：可调整字体颜色"));
    ui->modeDescLabel3->setText(QStringLiteral("关怀模式：放大字体，关闭调色功能"));

    ui->cacheLabel->setText(QStringLiteral("缓存管理"));
    ui->chatRecordGroup->setTitle(QStringLiteral("聊天记录"));
    ui->selectAllChatBtn->setText(QStringLiteral("全选"));
    ui->deleteChatBtn->setText(QStringLiteral("删除选中"));
    ui->medicalRecordGroup->setTitle(QStringLiteral("病例记录"));
    ui->selectAllMedicalBtn->setText(QStringLiteral("全选"));
    ui->deleteMedicalBtn->setText(QStringLiteral("删除选中"));

    ui->serverLabel->setText(QStringLiteral("服务器配置"));
    ui->ipLabel->setText(QStringLiteral("服务器 IP："));
    ui->portLabel->setText(QStringLiteral("端口号："));
    ui->autoConnectCheck->setText(QStringLiteral("登录成功后自动连接"));
    ui->saveServerBtn->setText(QStringLiteral("保存配置"));
}

void SettingsWidget::setupAboutPage()
{
    QPushButton *navAboutBtn = new QPushButton(QStringLiteral("关于我们"), this);
    navAboutBtn->setObjectName(QStringLiteral("navAboutBtn"));
    navAboutBtn->setCheckable(true);
    if (QHBoxLayout *navLayout = qobject_cast<QHBoxLayout *>(ui->navLayout)) {
        navLayout->insertWidget(4, navAboutBtn);
    }
    connect(navAboutBtn, &QPushButton::clicked, this, &SettingsWidget::onNavAboutClicked);

    QWidget *aboutPage = new QWidget(this);
    aboutPage->setObjectName(QStringLiteral("aboutPage"));
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setContentsMargins(36, 32, 36, 32);
    aboutLayout->setSpacing(18);

    QLabel *titleLabel = new QLabel(QStringLiteral("关于我们"), aboutPage);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    QLabel *imageLabel = new QLabel(aboutPage);
    imageLabel->setAlignment(Qt::AlignCenter);
    QPixmap pixmap(ResourcePaths::findPhoto("us.png"));
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap.scaled(420, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        imageLabel->setText(QStringLiteral("未找到图片"));
    }

    QLabel *descTitleLabel = new QLabel(QStringLiteral("创伤小组"), aboutPage);
    descTitleLabel->setAlignment(Qt::AlignCenter);
    QFont descTitleFont = descTitleLabel->font();
    descTitleFont.setPointSize(18);
    descTitleFont.setBold(true);
    descTitleLabel->setFont(descTitleFont);

    QLabel *descLabel = new QLabel(QStringLiteral(
        "七分钟救命，不然退款。这就是创伤小组的保证，投保人在小巷子里血流不止的时候，这种保证格外温暖人心。假设你没有因为失血过多而昏迷，计算着生命最后一刻的每分每秒…不用担心，因为援兵马上就到。首先，你会看到一辆重型装甲浮空车从天而降，用重机枪炮塔把那些想要杀你王八蛋统统撂倒。然后你会看到你的守护天使：一身白绿相间、武装到牙齿医护人员。等创伤小组的人员把你救回来，你会收到需要个人支付的账单。上面可能有一长串的零，但你还是会一脸带笑，再续签半年的保险。\n\n"
        "创伤小组在全球各大城市提供医疗、护理和撤离服务，备受经济实力富裕人士的高度推崇和追捧。实际上，他们可能是唯一一家公信力如此之高的公司。他们不碰政治，也不会多问。无论情况多么险峻，只要你及时付款，生命就可以获得保障。"),
        aboutPage);
    descLabel->setAlignment(Qt::AlignTop);
    descLabel->setWordWrap(true);
    descLabel->setTextFormat(Qt::PlainText);

    aboutLayout->addWidget(titleLabel);
    aboutLayout->addWidget(imageLabel, 1);
    aboutLayout->addWidget(descTitleLabel);
    aboutLayout->addWidget(descLabel);

    ui->pageStack->addWidget(aboutPage);
}

void SettingsWidget::rebuildModePageLayout()
{
    ui->modeLayout->setContentsMargins(32, 28, 32, 28);
    ui->modeLayout->setSpacing(16);
    ui->colorPreview->setMinimumHeight(132);
    ui->colorPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->bgStyleHintLabel->setWordWrap(true);
    ui->modeDescLabel->setWordWrap(true);
    ui->modeDescLabel2->setWordWrap(true);
    ui->modeDescLabel3->setWordWrap(true);

    auto *previewGroup = new QGroupBox(QStringLiteral("界面预览"), ui->modePage);
    auto *previewLayout = new QVBoxLayout(previewGroup);
    previewLayout->setContentsMargins(16, 18, 16, 16);
    previewLayout->addWidget(ui->colorPreview);

    auto *speechGroup = new QGroupBox(QStringLiteral("朗读设置"), ui->modePage);
    auto *speechGroupLayout = new QVBoxLayout(speechGroup);
    speechGroupLayout->setContentsMargins(16, 18, 16, 16);
    speechGroupLayout->setSpacing(12);
    ui->modeLayout->removeItem(ui->speechLayout);
    ui->modeLayout->removeItem(ui->rateLayout);
    speechGroupLayout->addLayout(ui->speechLayout);
    speechGroupLayout->addLayout(ui->rateLayout);

    ui->modeLayout->removeWidget(ui->colorPreview);
    ui->modeLayout->insertWidget(4, previewGroup);
    ui->modeLayout->insertWidget(5, speechGroup);

    if (ui->modePage->layout()) {
        ui->modePage->layout()->activate();
    }
}

QString SettingsWidget::historyRootDir() const
{
    const QString historyDir = QCoreApplication::applicationDirPath() + "/chat_history";
    QDir().mkpath(historyDir);
    return historyDir;
}

QString SettingsWidget::sanitizeHistoryUserName(const QString &username) const
{
    QString safeName = username.trimmed();
    if (safeName.isEmpty()) {
        safeName = QStringLiteral("anonymous");
    }
    safeName.replace(QRegularExpression(QStringLiteral(R"([\\/:*?"<>|\s]+)")), QStringLiteral("_"));
    return safeName;
}

QString SettingsWidget::currentUserHistoryDir() const
{
    const QString historyDir = historyRootDir() + "/" + sanitizeHistoryUserName(m_currentUsername);
    QDir().mkpath(historyDir);
    return historyDir;
}

QString SettingsWidget::userScopedKey(const QString &field) const
{
    return QStringLiteral("user/%1/%2").arg(sanitizeHistoryUserName(m_currentUsername), field);
}

QString SettingsWidget::currentUserRecordDir() const
{
    const QString recordDir = QDir::homePath() + "/SmartMedica/records/" + sanitizeHistoryUserName(m_currentUsername);
    QDir().mkpath(recordDir);
    return recordDir;
}

void SettingsWidget::setUsername(const QString &username)
{
    m_currentUsername = username;
    ui->usernameLabel->setText(username);
    loadSettings();
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
    updateNavChecks(pageIndex);

    if (pageIndex == 2) {
        loadChatRecords();
        loadMedicalRecords();
    }
}

void SettingsWidget::updateNavChecks(int pageIndex)
{
    ui->navUserInfoBtn->setChecked(pageIndex == 0);
    ui->navModeBtn->setChecked(pageIndex == 1);
    ui->navCacheBtn->setChecked(pageIndex == 2);
    ui->navServerBtn->setChecked(pageIndex == 3);
    if (QPushButton *navAboutBtn = findChild<QPushButton *>(QStringLiteral("navAboutBtn"))) {
        navAboutBtn->setChecked(pageIndex == 4);
    }
}

void SettingsWidget::loadSettings()
{
    const QString mode = m_settings->value("mode", QStringLiteral("普通模式")).toString();
    const QString bgColor = ThemeHelpers::normalizeBgColor(m_settings->value("bgColor", "#07111F").toString());
    const QString color = m_settings->value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
    const QString ip = m_settings->value("serverIP", "127.0.0.1").toString();
    const quint16 port = m_settings->value("serverPort", 9999).toUInt();
    const bool autoConnect = m_settings->value("autoConnect", true).toBool();
    const QString gender = m_settings->value(userScopedKey(QStringLiteral("gender")), QStringLiteral("保密")).toString();
    const int age = m_settings->value(userScopedKey(QStringLiteral("age")), 0).toInt();
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
        QLineEdit, QComboBox, QSpinBox {
            background: %6;
            color: %2;
            border: 1px solid rgba(0, 229, 255, 0.22);
            border-radius: 14px;
            padding: 8px 12px;
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
    m_settings->setValue(userScopedKey(QStringLiteral("gender")), ui->genderCombo->currentText());
    m_settings->setValue(userScopedKey(QStringLiteral("age")), ui->ageSpinBox->value());
    m_settings->sync();
    QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("用户信息已保存。"));
}

void SettingsWidget::loadChatRecords()
{
    ui->chatRecordList->clear();
    QDir dir(currentUserHistoryDir());
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
                if (parts.size() >= 3) {
                    displayText = parts[2].left(30);
                    if (parts[2].length() > 30) {
                        displayText += "...";
                    }
                }
            }
        }

        auto *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, fileInfo.fileName());
        ui->chatRecordList->addItem(item);
    }
}

void SettingsWidget::loadMedicalRecords()
{
    ui->medicalRecordList->clear();
    const QString recordDir = currentUserRecordDir();
    QDir dir(recordDir);
    if (!dir.exists()) {
        return;
    }

    const QStringList files = dir.entryList(QStringList() << "record_*.txt", QDir::Files, QDir::Time);
    for (const QString &fileName : files) {
        auto *item = new QListWidgetItem(fileName);
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

void SettingsWidget::onNavAboutClicked()
{
    switchToPage(4);
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
    const QColor color = QColorDialog::getColor(QColor(m_currentColor), this, QStringLiteral("选择字体颜色"));
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

    QDir dir(currentUserHistoryDir());
    int deletedCount = 0;
    for (QListWidgetItem *item : selectedItems) {
        const QString fileName = item->data(Qt::UserRole).toString();
        if (dir.remove(fileName)) {
            ++deletedCount;
            delete item;
        }
    }

    loadChatRecords();
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
