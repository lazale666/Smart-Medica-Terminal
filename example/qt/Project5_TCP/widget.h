#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void connectService();
    void disConnectService();
    void connectError(QAbstractSocket::SocketError socketError);
    void readData();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    QByteArray recvBuffer; // 新增：接收缓冲区，解决TCP粘包问题
};
#endif // WIDGET_H