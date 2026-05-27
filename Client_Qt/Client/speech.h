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
#include <QProcess>
#include "http.h"

const QString baiduTokenUrl = "http://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=%1&client_secret=%2&";
const QString client_id = "4hmi2kFIDORBnD29tUFtfQCE";
const QString client_secret = "C6uqUfvqUcmjPZQkQPxcvETZJJtPpM5s";

const QString baiduSpeechUrl = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=%1&token=%2";
const QString baiduTtsUrl = "http://tsn.baidu.com/text2audio?tex=%1&lan=zh&cuid=%2&ctp=1&tok=%3";

class Speech
{
public:
    explicit Speech();
    QString speechIdentify(QString fileName);
    bool textToSpeech(const QString &text, const QString &outputFile = "tts.wav");
    bool playAudio(const QString &filePath);
private:
    QString getJsonValue(QByteArray ba, QString key);
    QString getAccessToken();
private:
    QString accessToken;
    QProcess *playerProcess;
};

#endif // SPEECH_H
