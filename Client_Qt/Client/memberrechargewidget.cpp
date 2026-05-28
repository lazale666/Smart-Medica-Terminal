#include "memberrechargewidget.h"
#include "ui_memberrechargewidget.h"
#include "facerecognizewidget.h"
#include "resourcepaths.h"
#include "themehelpers.h"

#include <QMessageBox>
#include <QPalette>
#include <QPixmap>
#include <QBrush>
MemberRechargeWidget::MemberRechargeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MemberRechargeWidget)
    , m_isMember(false)
    , m_lockedSize(1228, 694)
    , m_currentMode("普通模式")
    , m_currentBgColor("#07111F")
    , m_fontColor("#D8F7FF")
{
    ui->setupUi(this);
    m_bgPath = ResourcePaths::findPhoto("pay_money_background.png");

    setFixedSize(m_lockedSize);
    m_settings = new QSettings("SmartMedica", "Client", this);

    connect(ui->backBtn, &QPushButton::clicked, this, &MemberRechargeWidget::onBackBtnClicked);
    connect(ui->purchaseBtn, &QPushButton::clicked, this, &MemberRechargeWidget::onPurchaseBtnClicked);

    applyAppearance(m_currentMode, m_currentBgColor, m_fontColor);
    updateBackground();
}

MemberRechargeWidget::~MemberRechargeWidget()
{
    delete ui;
}

void MemberRechargeWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBackground();
}

void MemberRechargeWidget::setUsername(const QString &username)
{
    m_username = username;
    loadMemberStatus();
}

void MemberRechargeWidget::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    m_currentMode = mode;
    m_currentBgColor = ThemeHelpers::normalizeBgColor(bgColor);
    m_fontColor = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_currentBgColor) : fontColor;
    applyModeSettings(mode);
    updateTexts();
    updateCardStyle();
    updatePurchaseButtonStyle();
}

void MemberRechargeWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;
    if (mode == QStringLiteral("关怀模式")) {
        ui->backBtn->setMinimumHeight(56);
        ui->purchaseBtn->setMinimumHeight(70);
    } else {
        ui->backBtn->setMinimumHeight(40);
        ui->purchaseBtn->setMinimumHeight(50);
    }
}

void MemberRechargeWidget::loadMemberStatus()
{
    const QString key = QString("member_%1").arg(m_username);
    m_isMember = m_settings->value(key, false).toBool();
    updatePurchaseButtonStyle();
}

void MemberRechargeWidget::saveMemberStatus()
{
    const QString key = QString("member_%1").arg(m_username);
    m_settings->setValue(key, m_isMember);
    m_settings->sync();
}

void MemberRechargeWidget::updateTexts()
{
    const bool care = (m_currentMode == QStringLiteral("关怀模式"));
    const bool light = ThemeHelpers::isLightTheme(m_currentBgColor);
    const QString titleColor = light ? "#0F2740" : "#00E5FF";
    const QString accentColor = light ? "#157A52" : "#31FFB7";
    const QString descColor = light ? "#35546B" : "#D8F7FF";
    const QString priceColor = light ? "#D96D00" : "#FFCF5A";

    ui->titleLabel->setText(QString("<p align='center'><span style='font-size:%1pt; font-weight:700; color:%2;'>开通会员</span></p>")
                                .arg(care ? 30 : 24)
                                .arg(titleColor));
    ui->cardTitle->setText(QString("<p align='center'><span style='font-size:%1pt; font-weight:700; color:%2;'>医疗智能体会员</span></p>")
                               .arg(care ? 24 : 20)
                               .arg(accentColor));
    ui->cardDesc1->setText(QString("<p align='center'><span style='font-size:%1pt; color:%2;'>尊享名医在线咨询</span></p>")
                               .arg(care ? 18 : 14)
                               .arg(descColor));
    ui->cardDesc2->setText(QString("<p align='center'><span style='font-size:%1pt; color:%2;'>获取专属医疗服务</span></p>")
                               .arg(care ? 18 : 14)
                               .arg(descColor));
    ui->priceLabel->setText(QString("<p align='center'><span style='font-size:%1pt; font-weight:700; color:%2;'>￥99.00</span></p>")
                                .arg(care ? 34 : 28)
                                .arg(priceColor));
}

void MemberRechargeWidget::updateCardStyle()
{
    const bool light = ThemeHelpers::isLightTheme(m_currentBgColor);
    setStyleSheet("QWidget#MemberRechargeWidget { background: transparent; }");
    ui->cardWidget->setStyleSheet(QString(
        "QWidget#cardWidget { background: %1; border: 1px solid %2; border-radius: 22px; }")
                                      .arg(light ? "rgba(255, 255, 255, 0.90)" : "rgba(2, 9, 20, 0.82)",
                                           light ? "rgba(15, 39, 64, 0.18)" : "rgba(0, 229, 255, 0.42)"));
    ui->backBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 14px; padding: 8px 16px; font-weight: 700; }"
        "QPushButton:hover { background: %4; }")
                                    .arg(light ? "rgba(255, 255, 255, 0.94)" : "rgba(6, 24, 45, 0.92)",
                                         light ? "#0F2740" : "#D8F7FF",
                                         light ? "rgba(15, 39, 64, 0.18)" : "rgba(0, 229, 255, 0.62)",
                                         light ? "rgba(199, 244, 255, 0.96)" : "rgba(0, 229, 255, 0.18)"));
}

void MemberRechargeWidget::updatePurchaseButtonStyle()
{
    const bool light = ThemeHelpers::isLightTheme(m_currentBgColor);
    if (m_isMember) {
        ui->purchaseBtn->setText(QStringLiteral("已开通会员"));
        ui->purchaseBtn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: 1px solid rgba(234, 251, 255, 0.75); border-radius: 16px; padding: 15px; font-size: 16px; font-weight: 700; }"
            "QPushButton:hover { background-color: %3; }")
                                            .arg(light ? "#B7F0D3" : "#31FFB7",
                                                 light ? "#0F2740" : "#03111D",
                                                 light ? "#D2F7E5" : "#72FFD0"));
    } else {
        ui->purchaseBtn->setText(QStringLiteral("立即购买"));
        ui->purchaseBtn->setStyleSheet(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7); color: #03111D; border: 1px solid rgba(234, 251, 255, 0.75); border-radius: 16px; padding: 15px; font-size: 16px; font-weight: 700; }"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31FFB7, stop:1 #00E5FF); }"
            "QPushButton:pressed { background-color: #00E5FF; }");
    }
}

void MemberRechargeWidget::onBackBtnClicked()
{
    emit backToMenu();
}

void MemberRechargeWidget::onPurchaseBtnClicked()
{
    if (m_isMember) {
        QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("您已是会员，无需重复购买。"));
        return;
    }

    FaceRecognizeWidget *faceWidget = new FaceRecognizeWidget(m_username, nullptr);
    faceWidget->applyAppearance(m_currentMode, m_currentBgColor, m_fontColor);
    connect(faceWidget, &FaceRecognizeWidget::recognitionSuccess, [=]() {
        m_isMember = true;
        saveMemberStatus();
        loadMemberStatus();
        emit memberStatusChanged(true);
        QMessageBox::information(nullptr, QStringLiteral("恭喜"), QStringLiteral("会员开通成功。"));
    });
    connect(faceWidget, &FaceRecognizeWidget::backToRecharge, [=]() {
        faceWidget->close();
    });
    faceWidget->show();
    faceWidget->raise();
    faceWidget->activateWindow();
}

void MemberRechargeWidget::updateBackground()
{
    QPalette palette;
    if (m_bgPath.isEmpty()) {
        palette.setBrush(QPalette::Window, QBrush(QColor(ThemeHelpers::isLightTheme(m_currentBgColor) ? "#F5FBFF" : "#07111F")));
        setPalette(palette);
        setAutoFillBackground(true);
        return;
    }

    QPixmap background(m_bgPath);
    if (background.isNull()) {
        palette.setBrush(QPalette::Window, QBrush(QColor(ThemeHelpers::isLightTheme(m_currentBgColor) ? "#F5FBFF" : "#07111F")));
    } else {
        palette.setBrush(QPalette::Window, QBrush(background.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    }
    setPalette(palette);
    setAutoFillBackground(true);
}
