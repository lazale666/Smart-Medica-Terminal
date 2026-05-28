#ifndef MEDICALRECORDWIDGET_H
#define MEDICALRECORDWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QDateTime>
#include <QListWidgetItem>
#include "recorddetailwidget.h"
#include "settingswidget.h"

namespace Ui {
class MedicalRecordWidget;
}

class MedicalRecordWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MedicalRecordWidget(QWidget *parent = nullptr);
    ~MedicalRecordWidget();

    void setServerInfo(const QString &ip, int port);
    void setUsername(const QString &username);
    void applyModeSettings(const QString &mode);
    void applyFontColor(const QString &color);
    void applyBgColor(const QString &color);

signals:
    void backToMenu();

private slots:
    void onAiFillBtnClicked();
    void onSaveBtnClicked();
    void onBackBtnClicked();
    void onRecordListClicked(QListWidgetItem *item);
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSettingsBtnClicked();
    void onModeChanged(const QString &mode);
    void onFontColorChanged(const QString &color);
    void onBgColorChanged(const QString &color);

private:
    void refreshRecordList();
    void saveRecord(const QString &diseaseName, const QString &diagnosisDate, const QString &treatment);
    void loadRecord(const QString &fileName);
    void loadRecordData(const QString &fileName, QString &diseaseName, QString &diagnosisDate, QString &treatment);

    QString getRecordDir() const;

private:
    Ui::MedicalRecordWidget *ui;
    QString m_username;
    QString m_serverIP;
    int m_serverPort;
    QString m_currentMode;
    QTcpSocket *socket;
    QByteArray m_buffer;
    bool m_isAiThinking;
    RecordDetailWidget *m_detailWidget;
    SettingsWidget *settingsWidget;
    QString m_fontColor;
    QString m_bgColor;
};

#endif // MEDICALRECORDWIDGET_H
