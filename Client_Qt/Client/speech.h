#ifndef SPEECH_H
#define SPEECH_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostInfo>
#include <QFile>
#include <QMessageBox>
#include <QIODevice>
#include "http.h"

const QString baiduTokenUrl = "http://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=%1&client_secret=%2&";
const QString client_id = "";
const QString client_secret = "";

const QString baiduSpeechurl = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=%1&token=%2";

class Speech
{
public:
    explicit Speech();
    QString speechIdentify(QString fileName);
private:
    QString getJsonVale(QByteArray ba,QString key);
private:
    QString accessToken;
};

#endif // SPEECH_H
