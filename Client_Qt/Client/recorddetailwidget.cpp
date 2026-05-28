
#include "recorddetailwidget.h"
#include "ui_recorddetailwidget.h"
#include <QFont>

RecordDetailWidget::RecordDetailWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordDetailWidget),
    m_isNewRecord(false),
    m_currentMode("普通模式")
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());

    connect(ui->closeBtn, &QPushButton::clicked, this, &RecordDetailWidget::close);
}

RecordDetailWidget::~RecordDetailWidget()
{
    delete ui;
}

void RecordDetailWidget::setRecordData(const QString &diseaseName, const QString &diagnosisDate, const QString &treatment)
{
    ui->diseaseEdit->setText(diseaseName);
    ui->dateEdit->setDate(QDate::fromString(diagnosisDate, "yyyy-MM-dd"));
    ui->treatmentEdit->setPlainText(treatment);
}

void RecordDetailWidget::setReadOnly(bool readOnly)
{
    ui->diseaseEdit->setReadOnly(readOnly);
    ui->dateEdit->setReadOnly(readOnly);
    ui->treatmentEdit->setReadOnly(readOnly);
}

void RecordDetailWidget::applyModeSettings(const QString &mode)
{
    m_currentMode = mode;
    
    QFont font = ui->diseaseEdit->font();
    QFont labelFont = ui->dateEdit->font();
    QFont btnFont = ui->closeBtn->font();
    QFont titleFont = ui->titleLabel->font();
    
    if (mode == "关怀模式") {
        font.setPointSize(font.pointSize() * 1.5);
        labelFont.setPointSize(labelFont.pointSize() * 1.5);
        btnFont.setPointSize(btnFont.pointSize() * 1.5);
        titleFont.setPointSize(titleFont.pointSize() * 1.5);
        
        ui->diseaseEdit->setFont(font);
        ui->dateEdit->setFont(labelFont);
        ui->treatmentEdit->setFont(font);
        ui->closeBtn->setFont(btnFont);
        ui->titleLabel->setFont(titleFont);
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
        ui->closeBtn->setFont(btnFont);
        ui->titleLabel->setFont(titleFont);
        ui->diseaseLabel->setFont(labelFont);
        ui->dateLabel->setFont(labelFont);
        ui->treatmentLabel->setFont(labelFont);
    }
}

void RecordDetailWidget::onSaveBtnClicked()
{
    QString diseaseName = ui->diseaseEdit->text().trimmed();
    QString diagnosisDate = ui->dateEdit->date().toString("yyyy-MM-dd");
    QString treatment = ui->treatmentEdit->toPlainText().trimmed();

    if (diseaseName.isEmpty()) {
        return;
    }

    emit saveRecord(diseaseName, diagnosisDate, treatment);
}
