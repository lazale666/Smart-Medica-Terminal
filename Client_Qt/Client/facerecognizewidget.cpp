#include "facerecognizewidget.h"
#include "ui_facerecognizewidget.h"
#include "themehelpers.h"

#include <QPixmap>

FaceRecognizeWidget::FaceRecognizeWidget(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FaceRecognizeWidget)
    , m_username(username)
    , m_currentMode("普通模式")
    , m_bgColor("#07111F")
    , m_fontColor("#D8F7FF")
{
    ui->setupUi(this);

    m_firstTimer = new QTimer(this);
    m_showImageTimer = new QTimer(this);
    m_successTimer = new QTimer(this);

    m_firstTimer->setSingleShot(true);
    m_showImageTimer->setSingleShot(true);
    m_successTimer->setSingleShot(true);

    connect(ui->backBtn, &QPushButton::clicked, this, &FaceRecognizeWidget::onBackBtnClicked);
    connect(m_firstTimer, &QTimer::timeout, this, &FaceRecognizeWidget::onTimerFirstTimeout);
    connect(m_showImageTimer, &QTimer::timeout, this, &FaceRecognizeWidget::onShowImageTimeout);

    applyAppearance(m_currentMode, m_bgColor, m_fontColor);
    ui->statusLabel->setText(QStringLiteral("正在进行人脸识别..."));
    m_firstTimer->start(3000);
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
        "QWidget#cardWidget { background: %2; border-radius: 15px; border: 2px solid %3; padding: 30px; }")
                      .arg(light
                               ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF)"
                               : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023)",
                           light ? "rgba(255,255,255,0.90)" : "rgba(2, 9, 20, 0.82)",
                           light ? "#7FD9FF" : "#00bcd4"));

    ui->backBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 10px; padding: 8px 16px; font-size: 14px; }"
        "QPushButton:hover { background: %4; }")
                                    .arg(light ? "rgba(255,255,255,0.96)" : "rgba(6,24,45,0.92)",
                                         light ? "#0F2740" : "#D8F7FF",
                                         light ? "rgba(15,39,64,0.18)" : "rgba(0,229,255,0.35)",
                                         light ? "rgba(199,244,255,0.96)" : "rgba(0,229,255,0.18)"));
    ui->statusLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 20pt; }").arg(light ? "#4C647A" : "#D8F7FF"));
}

void FaceRecognizeWidget::onTimerFirstTimeout()
{
    showFaceImage();
}

void FaceRecognizeWidget::showFaceImage()
{
    const QString imagePath = "D:/All Program/agant_example/Smart-Medica-Terminal/Client_Qt/Client/photo/face.png";
    QPixmap pixmap(imagePath);

    if (!pixmap.isNull()) {
        ui->faceLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->faceLabel->setStyleSheet(QString("QLabel { border: 2px solid %1; border-radius: 10px; }")
                                         .arg(ThemeHelpers::isLightTheme(m_bgColor) ? "#7FD9FF" : "#00bcd4"));
    }

    ui->statusLabel->setText(QStringLiteral("识别成功"));
    m_showImageTimer->start(3000);
}

void FaceRecognizeWidget::onShowImageTimeout()
{
    emit recognitionSuccess();
    QTimer::singleShot(500, [=]() {
        close();
    });
}

void FaceRecognizeWidget::onBackBtnClicked()
{
    emit backToRecharge();
}
