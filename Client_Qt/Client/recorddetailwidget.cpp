
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
    setStyleSheet(R"(
        QWidget#RecordDetailWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023);
        }
        QLabel#titleLabel {
            color: #00E5FF;
            font-weight: 700;
        }
        QTextEdit, QDateEdit {
            background: rgba(2, 9, 20, 0.86);
            color: #EAFBFF;
            border: 1px solid rgba(0, 229, 255, 0.38);
            border-radius: 14px;
            padding: 8px 12px;
        }
    )");
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
        resize(920, 760);
        font.setPointSize(16);
        labelFont.setPointSize(15);
        btnFont.setPointSize(15);
        titleFont.setPointSize(22);
        
        ui->diseaseEdit->setFont(font);
        ui->dateEdit->setFont(labelFont);
        ui->treatmentEdit->setFont(font);
        ui->closeBtn->setFont(btnFont);
        ui->titleLabel->setFont(titleFont);
        ui->diseaseLabel->setFont(labelFont);
        ui->dateLabel->setFont(labelFont);
        ui->treatmentLabel->setFont(labelFont);
        ui->closeBtn->setMinimumHeight(52);
        ui->diseaseEdit->setMinimumHeight(50);
        ui->treatmentEdit->setMinimumHeight(220);
    } else {
        resize(720, 560);
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
        ui->closeBtn->setMinimumHeight(36);
        ui->diseaseEdit->setMinimumHeight(0);
        ui->treatmentEdit->setMinimumHeight(0);
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
