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
#include <QTextToSpeech>
#include <functional>
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
    void setServerInfo(const QString &ip, int port, bool autoConnect);
    void applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor);
    void applyModeSettings(const QString &mode);
    void applyFontColor(const QString &color);
    void applyBgColor(const QString &color);
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
    void onLogout();
    void onModeChanged(const QString &mode);
    void onFontColorChanged(const QString &color);
    void onBgColorChanged(const QString &color);
    void onCacheCleared();
    void onReadBtnClicked();
    void onSettingsBtnClicked();
    void onCloseSettings();
    void onServerConfigChanged(const QString &ip, quint16 port, bool autoConnect);
    void onLogoutFromSettings();
    void onSpeechSettingsChanged(double volume, double rate);

signals:
    void backToMenu();
    void logout();
    void sendInfo(int count);
    void modeChanged(const QString &mode);
    void appearanceChanged(const QString &mode, const QString &bgColor, const QString &fontColor);

private:
    void loadSettings();
    void saveSettings();
    void createNewChat();
    void saveChatMessage(const QString &role, const QString &content);
    void loadChatHistory(const QString &fileName);
    void refreshHistoryList();
    QString getHistoryDir() const;
    void appendChatMessage(const QString &sender, const QString &message, bool isSelf);
    void appendSystemMessage(const QString &message);
    void appendHistorySeparator(bool isTop);
    void setSocketHandlersActive(bool active);
    void leaveChatScene(const std::function<void()> &afterCleanup);

private:
    Ui::Widget *ui;
    SettingsWidget *settingsWidget;
    QTcpSocket *socket;
    QTimer *timer;
    QTimer *historyTimer;
    QMessageBox *msg;
    Dialog *dia;
    Audio *audio;
    Speech *speech;
    QTextToSpeech *m_speech;
    QByteArray buffer;
    QString m_currentChatFile;
    QString m_username;
    QString m_serverIP;
    int m_serverPort;
    bool m_autoConnect;
    QString m_currentMode;
    QString m_fontColor;
    QString m_bgColor;
    QSettings *m_settings;
    int conFlag, errFlag, count;
    bool isRecording, m_isThinking, m_isInterrupted;
    bool m_isNewSession;
    QString m_firstMessage;
};

#endif // WIDGET_H
