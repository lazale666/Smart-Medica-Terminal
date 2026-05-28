#ifndef SETTINGSWIDGET_DOC_H
#define SETTINGSWIDGET_DOC_H

#include <QWidget>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWidget_Doc; }
QT_END_NAMESPACE

class SettingsWidget_Doc : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget_Doc(QWidget *parent = nullptr);
    ~SettingsWidget_Doc();

    void setUsername(const QString &username);
    void setServerConfig(const QString &ip, quint16 port);

signals:
    void logout();
    void serverConfigChanged(const QString &ip, quint16 port);
    void closeSettings();

private slots:
    void onLogoutBtnClicked();
    void onNavUserInfoClicked();
    void onNavCacheClicked();
    void onNavServerClicked();
    void onSaveServerBtnClicked();
    void onSaveUserInfoBtnClicked();
    void onClearChatHistoryClicked();

private:
    Ui::SettingsWidget_Doc *ui;
    QSettings *m_settings;
    QString m_currentUsername;

    void switchToPage(int pageIndex);
    void loadSettings();
    void saveSettings();
    void clearChatHistory();
};

#endif // SETTINGSWIDGET_DOC_H
