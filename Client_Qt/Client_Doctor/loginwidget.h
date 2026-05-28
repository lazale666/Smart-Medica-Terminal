#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QResizeEvent>
#include <QWidget>

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void loginSuccess(const QString &username);
    void localLoginSuccess(const QString &username, const QString &password);

private slots:
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();

private:
    Ui::LoginWidget *ui;
    QString m_username;
    QString m_password;
    QString m_bgPath;

    void updateBackground();
    bool verifyLocalCredentials(const QString &username, const QString &password);
    bool registerLocalUser(const QString &username, const QString &password);
};

#endif // LOGINWIDGET_H
