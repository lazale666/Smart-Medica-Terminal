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
    void setServerInfo(const QString &ip, int port);
    void disconnectFromServer();
    void applyFontColor(const QString &color);
    void applyBgColor(const QString &color);

signals:
    void openChat(const QString &serverIP, int serverPort, bool autoConnect);
    void openMedicalRecord(const QString &serverIP, int serverPort, bool autoConnect);
    void logout();

private slots:
    void onChatBtnClicked();
    void onMedicalRecordBtnClicked();
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
};

#endif // MENUWIDGET_H
