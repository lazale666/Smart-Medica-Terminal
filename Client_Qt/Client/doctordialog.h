#ifndef DOCTORDIALOG_H
#define DOCTORDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

namespace Ui {
class DoctorDialog;
}

class DoctorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoctorDialog(QTcpSocket *socket, const QString &username, const QString &doctorName, QWidget *parent = nullptr);
    ~DoctorDialog();

private slots:
    void readData();
    void onSendBtnClicked();
    void onCloseBtnClicked();

private:
    Ui::DoctorDialog *ui;
    QTcpSocket *m_socket;
    QString m_username;
    QString m_doctorName;

    void sendMessage(const QString &message);
};

#endif // DOCTORDIALOG_H