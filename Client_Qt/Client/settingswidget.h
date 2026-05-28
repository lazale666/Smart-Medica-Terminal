#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWidget; }
QT_END_NAMESPACE

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

    void setUsername(const QString &username);
    void setServerConfig(const QString &ip, quint16 port, bool autoConnect);
    void setCurrentMode(const QString &mode);
    void setFontColor(const QString &color);
    void setBgColor(const QString &color);

signals:
    void logout();
    void serverConfigChanged(const QString &ip, quint16 port, bool autoConnect);
    void modeChanged(const QString &mode);
    void fontColorChanged(const QString &color);
    void bgColorChanged(const QString &color);
    void closeSettings();
    void cacheCleared();
    void connectNow(const QString &ip, quint16 port);
    void speechSettingsChanged(double volume, double rate);

private slots:
    void onLogoutBtnClicked();
    void onModeChanged(int index);
    void onColorBtnClicked();
    void onNavUserInfoClicked();
    void onNavModeClicked();
    void onNavCacheClicked();
    void onNavServerClicked();
    void onCloseBtnClicked();
    void onSelectAllChatClicked();
    void onDeleteChatClicked();
    void onSelectAllMedicalClicked();
    void onDeleteMedicalClicked();
    void onBgStyleChanged(int index);
    void onSaveServerBtnClicked();
    void onSaveUserInfoBtnClicked();
    void onVolumeSliderChanged(int value);
    void onRateSliderChanged(int value);

private:
    Ui::SettingsWidget *ui;
    QSettings *m_settings;
    QString m_currentUsername;
    QString m_currentColor;
    QString m_currentBgColor;

    void switchToPage(int pageIndex);
    void loadSettings();
    void saveSettings();
    void loadChatRecords();
    void loadMedicalRecords();
    void updateTheme();
    void deleteSelectedChatRecords();
    void deleteSelectedMedicalRecords();
};

#endif // SETTINGSWIDGET_H
