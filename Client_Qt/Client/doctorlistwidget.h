#ifndef DOCTORLISTWIDGET_H
#define DOCTORLISTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QListWidgetItem>

namespace Ui {
class DoctorListWidget;
}

class DoctorListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorListWidget(const QString &serverIP, int serverPort, const QString &username, QWidget *parent = nullptr);
    ~DoctorListWidget();

signals:
    void backToMenu();

private slots:
    void connectToServer();
    void readData();
    void requestDoctorList();
    void onBackBtnClicked();

private:
    Ui::DoctorListWidget *ui;
    QTcpSocket *socket;
    QString m_serverIP;
    int m_serverPort;
    QString m_username;
    QTimer *refreshTimer;
    QByteArray m_buffer;

    void sendRequest(const QJsonObject &obj);
};

#endif // DOCTORLISTWIDGET_H
