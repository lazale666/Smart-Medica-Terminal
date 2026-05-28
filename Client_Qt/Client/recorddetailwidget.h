#ifndef RECORDDETAILWIDGET_H
#define RECORDDETAILWIDGET_H

#include <QWidget>
#include <QString>

namespace Ui {
class RecordDetailWidget;
}

class RecordDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordDetailWidget(QWidget *parent = nullptr);
    ~RecordDetailWidget();

    void setRecordData(const QString &diseaseName, const QString &diagnosisDate, const QString &treatment);
    void setReadOnly(bool readOnly);

signals:
    void saveRecord(const QString &diseaseName, const QString &diagnosisDate, const QString &treatment);

private slots:
    void onSaveBtnClicked();

private:
    Ui::RecordDetailWidget *ui;
    QString m_fileName;
    bool m_isNewRecord;
};

#endif // RECORDDETAILWIDGET_H
