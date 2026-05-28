
#include "recorddetailwidget.h"
#include "ui_recorddetailwidget.h"

RecordDetailWidget::RecordDetailWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordDetailWidget),
    m_isNewRecord(false)
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
