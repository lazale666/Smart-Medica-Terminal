#ifndef MEMBERRECHARGEWIDGET_H
#define MEMBERRECHARGEWIDGET_H

#include <QWidget>
#include <QSettings>

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

signals:
    void backToMenu();

private slots:
    void onBackBtnClicked();
    void onPurchaseBtnClicked();

private:
    Ui::MemberRechargeWidget *ui;
    QSettings *m_settings;
    QString m_username;
    bool m_isMember;

    void loadMemberStatus();
    void saveMemberStatus();
};

#endif // MEMBERRECHARGEWIDGET_H
