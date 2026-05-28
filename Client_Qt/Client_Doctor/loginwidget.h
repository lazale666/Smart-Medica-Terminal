#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QResizeEvent>

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
    void loginSuccessWithSocket(const QString &username, QTcpSocket *socket);

private slots:
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();
    void on_clearBtn_clicked();
    void sendLoginRequest();
    void readData();

private:
    Ui::LoginWidget *ui;
    QTcpSocket *socket;
    QString m_username;
    QString m_password;
    QString m_bgPath;
    QByteArray m_buffer;

    void sendRequest(const QJsonObject &obj);
    void updateBackground();
    bool verifyLocalCredentials(const QString &username, const QString &password);
    bool registerLocalUser(const QString &username, const QString &password);
};

#endif // LOGINWIDGET_H
