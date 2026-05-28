#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QSettings>

namespace Ui {
class MenuWidget;
}

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

    void setUsername(const QString &username);
    QString getUsername() const;
    void setServerInfo(const QString &ip, int port);
    void disconnectFromServer();
    void ensureServerConnected();
    void refreshMemberAccessState();
    void applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor);
    void applyModeSettings(const QString &mode);
    void applyFontColor(const QString &color);
    void applyBgColor(const QString &color);

signals:
    void openChat(const QString &serverIP, int serverPort, bool autoConnect);
    void openMedicalRecord(const QString &serverIP, int serverPort, bool autoConnect);
    void openDoctorChat(const QString &serverIP, int serverPort);
    void openMemberRecharge();
    void modeChanged(const QString &mode);
    void appearanceChanged(const QString &mode, const QString &bgColor, const QString &fontColor);
    void logout();

private slots:
    void onChatBtnClicked();
    void onMedicalRecordBtnClicked();
    void onDoctorChatBtnClicked();
    void onMemberRechargeBtnClicked();
    void onSettingsBtnClicked();
    void onLogoutBtnClicked();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void loadSettings();
    void saveSettings();
    void updateConnectionStatus();
    void connectToServer();
    bool isCurrentUserMember() const;
    void showMemberRequiredMessage();

private:
    Ui::MenuWidget *ui;
    QString m_username;
    QString m_serverIP;
    int m_serverPort;
    bool m_autoConnect;
    QTcpSocket *socket;
    QSettings *m_settings;
    QString m_fontColor;
    QString m_bgColor;
    QString m_currentMode;
};

#endif // MENUWIDGET_H
