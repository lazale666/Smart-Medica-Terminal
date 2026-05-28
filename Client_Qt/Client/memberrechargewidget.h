#ifndef MEMBERRECHARGEWIDGET_H
#define MEMBERRECHARGEWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QResizeEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MemberRechargeWidget; }
QT_END_NAMESPACE

class MemberRechargeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemberRechargeWidget(QWidget *parent = nullptr);
    ~MemberRechargeWidget();

    void setUsername(const QString &username);
    void applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor);
    void applyModeSettings(const QString &mode);

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void backToMenu();
    void memberStatusChanged(bool isMember);

private slots:
    void onBackBtnClicked();
    void onPurchaseBtnClicked();

private:
    Ui::MemberRechargeWidget *ui;
    QSettings *m_settings;
    QString m_username;
    bool m_isMember;
    QString m_bgPath;
    QSize m_lockedSize;
    QString m_currentMode;
    QString m_currentBgColor;
    QString m_fontColor;

    void loadMemberStatus();
    void saveMemberStatus();
    void updateTexts();
    void updateCardStyle();
    void updatePurchaseButtonStyle();
    void updateBackground();
};

#endif // MEMBERRECHARGEWIDGET_H
