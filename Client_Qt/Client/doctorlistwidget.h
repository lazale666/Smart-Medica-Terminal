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

class DoctorDialog;

struct DoctorListEntry
{
    qint64 id;
    QString name;
    bool online;
    int activeClients = 0;
};

class DoctorListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorListWidget(const QString &serverIP, int serverPort, const QString &username, QWidget *parent = nullptr);
    ~DoctorListWidget();
    void applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor);
    void applyModeSettings(const QString &mode);

signals:
    void backToMenu();

public slots:
    void closeActiveWindowsAndReturn();

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
    QString m_bgColor;
    QString m_fontColor;
    QTimer *refreshTimer;
    QByteArray m_buffer;
    QVector<DoctorListEntry> m_doctors;
    bool m_isConnecting = false;
    bool m_hasActiveSession = false;
    qint64 m_selectedDoctorId = -1;
    DoctorDialog *m_activeDialog = nullptr;

    void sendRequest(const QJsonObject &obj);
    void renderDoctorList();
    void requestOnlineDoctors();
    void setDoctorSelectionEnabled(bool enabled);
};

#endif // DOCTORLISTWIDGET_H
