#include "facerecognizewidget.h"
#include "ui_facerecognizewidget.h"
#include "resourcepaths.h"
#include "themehelpers.h"

#include <QPixmap>

FaceRecognizeWidget::FaceRecognizeWidget(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FaceRecognizeWidget)
    , m_username(username)
    , m_currentMode(QStringLiteral("普通模式"))
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
{
    ui->setupUi(this);
    setupUI();

    m_firstTimer = new QTimer(this);
    m_showImageTimer = new QTimer(this);
    m_successTimer = new QTimer(this);

    m_firstTimer->setSingleShot(true);
    m_showImageTimer->setSingleShot(true);
    m_successTimer->setSingleShot(true);

    connect(ui->backBtn, &QPushButton::clicked, this, &FaceRecognizeWidget::onBackBtnClicked);
    connect(ui->startBtn, &QPushButton::clicked, this, &FaceRecognizeWidget::onStartBtnClicked);
    connect(m_firstTimer, &QTimer::timeout, this, &FaceRecognizeWidget::onTimerFirstTimeout);
    connect(m_showImageTimer, &QTimer::timeout, this, &FaceRecognizeWidget::onShowImageTimeout);

    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
    ui->statusLabel->setText(QStringLiteral("请确认开始识别，确认后将进行 3 秒本地人脸识别。"));
}

FaceRecognizeWidget::~FaceRecognizeWidget()
{
    delete ui;
}

void FaceRecognizeWidget::applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor)
{
    m_currentMode = mode;
    m_bgColor = ThemeHelpers::normalizeBgColor(bgColor);
    m_fontColor = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(m_bgColor) : fontColor;
    const bool light = ThemeHelpers::isLightTheme(m_bgColor);

    setStyleSheet(QString(
        "QWidget#FaceRecognizeWidget { background: %1; }"
        "QWidget#cardWidget { background: %2; border-radius: 24px; border: 2px solid %3; padding: 36px; }"
        "QLabel#faceLabel { background: %4; border-radius: 18px; border: 2px dashed %3; padding: 18px; }")
                      .arg(light
                               ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF)"
                               : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023)",
                           light ? "rgba(255,255,255,0.92)" : "rgba(2, 9, 20, 0.84)",
                           light ? "#2AA8D8" : "#00E5FF",
                           light ? "rgba(245,251,255,0.98)" : "rgba(6,24,45,0.96)"));

    const QString secondaryButton = light ? "rgba(255,255,255,0.96)" : "rgba(6,24,45,0.92)";
    const QString secondaryText = light ? "#0F2740" : "#D8F7FF";
    const QString primaryHover = light ? "rgba(199,244,255,0.96)" : "rgba(49,255,183,0.88)";

    ui->backBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 12px; padding: 10px 18px; font: 700 14px \"Microsoft YaHei\"; }"
        "QPushButton:hover { background: %4; }")
                                    .arg(secondaryButton,
                                         secondaryText,
                                         light ? "rgba(15,39,64,0.18)" : "rgba(0,229,255,0.35)",
                                         light ? "rgba(199,244,255,0.96)" : "rgba(0,229,255,0.18)"));

    ui->startBtn->setStyleSheet(QString(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7); color: #03111D; border: 1px solid rgba(234,251,255,0.75); border-radius: 14px; padding: 12px 20px; font: 700 15px \"Microsoft YaHei\"; }"
        "QPushButton:hover { background: %1; }"
        "QPushButton:disabled { background: rgba(120,140,150,0.45); color: rgba(255,255,255,0.65); }")
                                     .arg(primaryHover));

    ui->statusLabel->setStyleSheet(QString("QLabel { color: %1; font: 700 20pt \"Microsoft YaHei\"; }")
                                       .arg(light ? "#0F2740" : "#EAFBFF"));
    ui->faceLabel->setStyleSheet(QString("QLabel { color: %1; font: 600 12pt \"Microsoft YaHei\"; }")
                                     .arg(light ? "#4C647A" : "#8BB9C8"));
}

void FaceRecognizeWidget::setupUI()
{
    setWindowFlag(Qt::Window, true);
    setWindowFlag(Qt::WindowCloseButtonHint, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMinimumSize(720, 760);
    ui->backBtn->setText(QStringLiteral("返回"));
    ui->startBtn->setText(QStringLiteral("确认开始识别"));
    ui->statusLabel->setAlignment(Qt::AlignCenter);
    ui->statusLabel->setWordWrap(true);
    ui->faceLabel->setAlignment(Qt::AlignCenter);
    ui->faceLabel->setText(QStringLiteral("点击下方按钮后开始识别"));
}

void FaceRecognizeWidget::onStartBtnClicked()
{
    ui->startBtn->setEnabled(false);
    ui->statusLabel->setText(QStringLiteral("正在进行人脸识别，请保持正对摄像头"));
    m_firstTimer->start(3000);
}

void FaceRecognizeWidget::onTimerFirstTimeout()
{
    showFaceImage();
}

void FaceRecognizeWidget::showFaceImage()
{
    const QString imagePath = ResourcePaths::findPhoto("face.png");
    QPixmap pixmap(imagePath);

    if (!pixmap.isNull()) {
        ui->faceLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->faceLabel->setPixmap(QPixmap());
        ui->faceLabel->setText(QStringLiteral("未找到识别图片"));
    }

    showSuccessMessage();
    m_showImageTimer->start(3000);
}

void FaceRecognizeWidget::showSuccessMessage()
{
    ui->statusLabel->setText(QStringLiteral("识别成功，正在为你开通会员服务"));
}

void FaceRecognizeWidget::onShowImageTimeout()
{
    emit recognitionSuccess();
    close();
}

void FaceRecognizeWidget::onBackBtnClicked()
{
    ui->startBtn->setEnabled(true);
    emit backToRecharge();
    close();
}
