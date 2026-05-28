#include "memberrechargewidget.h"
#include "ui_memberrechargewidget.h"
#include "facerecognizewidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QPalette>
#include <QPixmap>
#include <QBrush>

MemberRechargeWidget::MemberRechargeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MemberRechargeWidget)
    , m_isMember(false)
    , m_lockedSize(1228, 694)
    , m_currentMode("普通模式")
{
    ui->setupUi(this);
    m_bgPath = QCoreApplication::applicationDirPath() + "/photo/pay_money_background.png";
    if (!QFileInfo::exists(m_bgPath)) {
        m_bgPath = "D:/All Program/agant_example/Smart-Medica-Terminal/Client_Qt/Client/photo/pay_money_background.png";
    }

    setFixedSize(m_lockedSize);
    setStyleSheet(R"(
        QWidget#MemberRechargeWidget {
            background: transparent;
        }
        QWidget#cardWidget {
            background: rgba(2, 9, 20, 0.68);
            border: 1px solid rgba(0, 229, 255, 0.42);
            border-radius: 24px;
        }
        QPushButton#backBtn {
            background: rgba(6, 24, 45, 0.92);
            color: #D8F7FF;
            border: 1px solid rgba(0, 229, 255, 0.62);
            border-radius: 14px;
        }
    )");
    ui->cardWidget->setStyleSheet("QWidget#cardWidget { background: rgba(2, 9, 20, 0.82); border: 1px solid rgba(0, 229, 255, 0.42); border-radius: 22px; }");
    ui->backBtn->setStyleSheet("QPushButton { background: rgba(6, 24, 45, 0.92); color: #D8F7FF; border: 1px solid rgba(0, 229, 255, 0.62); border-radius: 14px; padding: 8px 16px; font-weight: 700; } QPushButton:hover { background: rgba(0, 229, 255, 0.18); }");
    ui->titleLabel->setText("<p align='center'><span style='font-size:24pt; font-weight:700; color:#00E5FF;'>开通会员</span></p>");
    ui->cardTitle->setText("<p align='center'><span style='font-size:20pt; font-weight:700; color:#31FFB7;'>医疗智能体会员</span></p>");
    ui->cardDesc1->setText("<p align='center'><span style='font-size:14pt; color:#D8F7FF;'>尊享名医在线咨询</span></p>");
    ui->cardDesc2->setText("<p align='center'><span style='font-size:14pt; color:#D8F7FF;'>获取专属医疗服务</span></p>");
    ui->priceLabel->setText("<p align='center'><span style='font-size:28pt; font-weight:700; color:#FFCF5A;'>￥99.00</span></p>");
    updateBackground();

    m_settings = new QSettings("SmartMedica", "Client", this);

    connect(ui->backBtn, &QPushButton::clicked, this, &MemberRechargeWidget::onBackBtnClicked);
    connect(ui->purchaseBtn, &QPushButton::clicked, this, &MemberRechargeWidget::onPurchaseBtnClicked);
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

void MemberRechargeWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;

    if (mode == "关怀模式") {
        ui->titleLabel->setText("<p align='center'><span style='font-size:30pt; font-weight:700; color:#00E5FF;'>开通会员</span></p>");
        ui->cardTitle->setText("<p align='center'><span style='font-size:24pt; font-weight:700; color:#31FFB7;'>医疗智能体会员</span></p>");
        ui->cardDesc1->setText("<p align='center'><span style='font-size:18pt; color:#D8F7FF;'>尊享名医在线咨询</span></p>");
        ui->cardDesc2->setText("<p align='center'><span style='font-size:18pt; color:#D8F7FF;'>获取专属医疗服务</span></p>");
        ui->priceLabel->setText("<p align='center'><span style='font-size:34pt; font-weight:700; color:#FFCF5A;'>￥99.00</span></p>");
        ui->backBtn->setMinimumHeight(56);
        ui->purchaseBtn->setMinimumHeight(70);
    } else {
        ui->titleLabel->setText("<p align='center'><span style='font-size:24pt; font-weight:700; color:#00E5FF;'>开通会员</span></p>");
        ui->cardTitle->setText("<p align='center'><span style='font-size:20pt; font-weight:700; color:#31FFB7;'>医疗智能体会员</span></p>");
        ui->cardDesc1->setText("<p align='center'><span style='font-size:14pt; color:#D8F7FF;'>尊享名医在线咨询</span></p>");
        ui->cardDesc2->setText("<p align='center'><span style='font-size:14pt; color:#D8F7FF;'>获取专属医疗服务</span></p>");
        ui->priceLabel->setText("<p align='center'><span style='font-size:28pt; font-weight:700; color:#FFCF5A;'>￥99.00</span></p>");
        ui->backBtn->setMinimumHeight(40);
        ui->purchaseBtn->setMinimumHeight(50);
    }
}

void MemberRechargeWidget::loadMemberStatus()
{
    QString key = QString("member_%1").arg(m_username);
    m_isMember = m_settings->value(key, false).toBool();
    
    if (m_isMember) {
        ui->purchaseBtn->setText("已开通会员");
        ui->purchaseBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #31FFB7;"
            "    color: #03111D;"
            "    border: 1px solid rgba(234, 251, 255, 0.75);"
            "    border-radius: 16px;"
            "    padding: 15px;"
            "    font-size: 16px;"
            "    font-weight: 700;"
            "}"
            "QPushButton:hover {"
            "    background-color: #72FFD0;"
            "}"
        );
    } else {
        ui->purchaseBtn->setText("立即购买");
        ui->purchaseBtn->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);"
            "    color: #03111D;"
            "    border: 1px solid rgba(234, 251, 255, 0.75);"
            "    border-radius: 16px;"
            "    padding: 15px;"
            "    font-size: 16px;"
            "    font-weight: 700;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #31FFB7, stop:1 #00E5FF);"
            "}"
            "QPushButton:pressed {"
            "    background-color: #00E5FF;"
            "}"
        );
    }
}

void MemberRechargeWidget::saveMemberStatus()
{
    QString key = QString("member_%1").arg(m_username);
    m_settings->setValue(key, m_isMember);
    m_settings->sync();
}

void MemberRechargeWidget::onBackBtnClicked()
{
    emit backToMenu();
}

void MemberRechargeWidget::onPurchaseBtnClicked()
{
    if (m_isMember) {
        QMessageBox::information(nullptr, "提示", "您已是会员，无法再次购买！");
        return;
    }

    FaceRecognizeWidget *faceWidget = new FaceRecognizeWidget(m_username, this);
    connect(faceWidget, &FaceRecognizeWidget::recognitionSuccess, [=]() {
        m_isMember = true;
        saveMemberStatus();
        loadMemberStatus();
        QMessageBox::information(nullptr, "恭喜", "会员开通成功！");
    });
    connect(faceWidget, &FaceRecognizeWidget::backToRecharge, [=]() {
        faceWidget->close();
    });
    faceWidget->show();
}

void MemberRechargeWidget::updateBackground()
{
    if (m_bgPath.isEmpty()) {
        return;
    }

    QPixmap background(m_bgPath);
    if (background.isNull()) {
        return;
    }

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(background.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    setPalette(palette);
    setAutoFillBackground(true);
}
