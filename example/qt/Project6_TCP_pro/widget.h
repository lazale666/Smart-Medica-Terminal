#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTimer>
#include "dialog.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void sendInfo(int count);

private slots:
    void on_pushButton_2_clicked();
    void connectService();
    void disConnectService();
    void connectError(QAbstractSocket::SocketError err);
    void readData();
    void reconnect();
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    QTimer *timer;
    Dialog *dia;

    int conFlag;
    int errflag;
    int count;
    QByteArray recvBuffer;
};

#endif // WIDGET_H