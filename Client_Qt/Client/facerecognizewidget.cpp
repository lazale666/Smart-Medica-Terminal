#include "facerecognizewidget.h"
#include "ui_facerecognizewidget.h"
#include <QPixmap>
#include <QDebug>

FaceRecognizeWidget::FaceRecognizeWidget(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FaceRecognizeWidget)
    , m_username(username)
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

    ui->statusLabel->setText("正在进行人脸识别...");

    m_firstTimer->start(3000);
}

FaceRecognizeWidget::~FaceRecognizeWidget()
{
    delete ui;
}

void FaceRecognizeWidget::onTimerFirstTimeout()
{
    showFaceImage();
}

void FaceRecognizeWidget::showFaceImage()
{
    QString imagePath = "D:/All Program/agant_example/Smart-Medica-Terminal/Client_Qt/Client/photo/face.png";
    QPixmap pixmap(imagePath);
    
    if (!pixmap.isNull()) {
        ui->faceLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->faceLabel->setStyleSheet("QLabel { border: 2px solid #00bcd4; border-radius: 10px; }");
    }
    
    ui->statusLabel->setText("识别成功");
    
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
