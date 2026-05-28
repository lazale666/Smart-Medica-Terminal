#ifndef DOCTORLISTWIDGET_H
#define DOCTORLISTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QListWidgetItem>
#include <QVector>

namespace Ui {
class DoctorListWidget;
}

struct DoctorListEntry
{
    qint64 id;
    QString name;
    bool online;
};

class DoctorListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorListWidget(const QString &serverIP, int serverPort, const QString &username, QWidget *parent = nullptr);
    ~DoctorListWidget();
    void applyModeSettings(const QString &mode);

signals:
    void backToMenu();

private slots:
    void connectToServer();
    void readData();
    void requestDoctorList();
    void onBackBtnClicked();
    void onDoctorItemClicked(QListWidgetItem *item);

private:
    Ui::DoctorListWidget *ui;
    QTcpSocket *socket;
    QString m_serverIP;
    int m_serverPort;
    QString m_username;
    QString m_currentMode;
    QTimer *refreshTimer;
    QByteArray m_buffer;
    QVector<DoctorListEntry> m_doctors;

    void sendRequest(const QJsonObject &obj);
    void renderDoctorList();
    void requestOnlineDoctors();
};

#endif // DOCTORLISTWIDGET_H
