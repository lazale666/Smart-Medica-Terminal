#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog();

    void setHistoryData(const QJsonArray &history);

private:
    Ui::HistoryDialog *ui;
};

#endif // HISTORYDIALOG_H