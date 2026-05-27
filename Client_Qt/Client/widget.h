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
#include <QFile>
#include <QCoreApplication>
#include "dialog.h"
#include "audio.h"
#include "speech.h"

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
    void onHistoryLoadTimerTick();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    QByteArray buffer;
    int conFlag,errFlag;
    QTimer *timer;
    QTimer *historyTimer;
    int count;
    QMessageBox *msg;
    Dialog *dia;
    Audio *audio;
    Speech *speech;
    bool isRecording;
    QString m_chatHistoryFile;
    bool m_isThinking;
    bool m_isInterrupted;
    QString m_pendingUserMessage;

    void saveChatMessage(const QString &role, const QString &content);
    void loadChatHistory();
};

#endif // WIDGET_H
