#ifndef DOCTORCHATWIDGET_H
#define DOCTORCHATWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include "historydialog.h"

namespace Ui {
class DoctorChatWidget;
}

class DoctorChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorChatWidget(QWidget *parent = nullptr);
    explicit DoctorChatWidget(QTcpSocket *existingSocket, QWidget *parent = nullptr);
    ~DoctorChatWidget();

private slots:
    void connectToServer();
    void readData();
    void on_sendBtn_clicked();
    void on_historyBtn_clicked();

private:
    void sendMessage(const QString &message);
    void initConnections();

private:
    Ui::DoctorChatWidget *ui;
    QTcpSocket *socket;
    bool m_externalSocket;
    char data[4];
    QStringList messageHistory;
    QJsonArray userHistory;
    HistoryDialog *historyDialog;
};

#endif // DOCTORCHATWIDGET_H