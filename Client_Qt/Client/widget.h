#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QSettings>
#include "dialog.h"
#include "audio.h"
#include "speech.h"
#include "settingswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void setUsername(const QString &username);
    void applyModeSettings(const QString &mode);
    void applyFontColor(const QString &color);
    void connectToServer();

private slots:
    void connectService();
    void disConnectService();
    void connectError(QAbstractSocket::SocketError err);
    void reconnect();
    void readData();
    void onHistoryLoadTimerTick();
    void on_pushButton_clicked();
    void on_voiceBtn_pressed();
    void on_voiceBtn_released();
    void onHistoryItemClicked(QListWidgetItem *item);
    void onSettingsBtnClicked();
    void onLogout();
    void onModeChanged(const QString &mode);
    void onFontColorChanged(const QString &color);
    void onServerConfigChanged(const QString &ip, quint16 port, bool autoConnect);
    void onCloseSettings();
    void onReadBtnClicked();
    void onCacheCleared();

signals:
    void sendInfo(int cnt);
    void logout();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    QTimer *timer;
    QTimer *historyTimer;
    QMessageBox *msg;
    Dialog *dia;
    Audio *audio;
    Speech *speech;
    SettingsWidget *settingsWidget;
    QByteArray buffer;
    QString m_currentChatFile;
    QString m_username;
    QString serverIP;
    quint16 serverPort;
    bool m_autoConnect;
    QString m_currentMode;
    QString m_fontColor;
    QSettings *m_settings;
    int conFlag, errFlag, count;
    bool isRecording, m_isThinking, m_isInterrupted;
    bool m_isNewSession;
    QString m_firstMessage;

    void saveChatMessage(const QString &role, const QString &content);
    void loadChatHistory(const QString &fileName);
    void createNewChat();
    void refreshHistoryList();
    QString getHistoryDir();
    void loadSettings();
};

#endif // WIDGET_H