#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTimer>
#include <QMouseEvent>
#include "dialog.h"
#include "audio.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void sendInfo(int);

private slots:
    void on_pushButton_2_clicked();
    void connectService();
    void disConnectService();
    void connectError(QAbstractSocket::SocketError);
    void readData();
    void reconnect();
    void on_pushButton_clicked();
    void on_voiceBtn_pressed();
    void on_voiceBtn_released();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    char data[4];
    int conFlag,errFlag;
    QTimer *timer;
    int count;
    QMessageBox *msg;
    Dialog *dia;
    Audio *audio;
    bool isRecording;
};

#endif // WIDGET_H