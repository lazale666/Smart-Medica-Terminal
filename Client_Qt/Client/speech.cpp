#include "speech.h"
#include <QUrl>
#include <QDebug>

Speech::Speech() : playerProcess(nullptr) {}

Speech::~Speech() {
    stopAudio();
}

void Speech::stopAudio() {
    if (playerProcess) {
        playerProcess->kill();
        playerProcess->waitForFinished(1000);
        playerProcess->deleteLater();
        playerProcess = nullptr;
    }
}

bool Speech::isPlaying() const {
    return playerProcess && playerProcess->state() == QProcess::Running;
}

QString Speech::getAccessToken()
{
    if (!accessToken.isEmpty()) {
        return accessToken;
    }

    QString tokenUrl = QString(baiduTokenUrl).arg(client_id).arg(client_secret);
    QMap<QString, QString> headers;
    QByteArray requestData;
    QByteArray replyData;
    Http httputil;
    bool success = httputil.postSyn(tokenUrl, headers, requestData, replyData);
    if (success) {
        accessToken = getJsonValue(replyData, "access_token");
        qDebug() << "Access Token obtained:" << accessToken;
    } else {
        accessToken = "";
    }
    return accessToken;
}

QString Speech::speechIdentify(QString fileName)
{
    QString token = getAccessToken();
    if (token.isEmpty()) {
        return "";
    }

    QString baiduSpeech = QString(baiduSpeechUrl).arg("LAPTOP-71LN9B3Q").arg(token);
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray requestData = file.readAll();
    file.close();

    QMap<QString, QString> headers;
    headers.insert("Content-Type", "audio/pcm;rate=16000");

    Http httputil;
    QByteArray replyData;
    bool result = httputil.postSyn(baiduSpeech, headers, requestData, replyData);
    if (result) {
        QString text = getJsonValue(replyData, "result");
        return text;
    } else {
        QMessageBox::warning(NULL, "识别提示", "识别失败");
    }
    return "";
}

bool Speech::textToSpeech(const QString &text, const QString &outputFile)
{
    QString token = getAccessToken();
    if (token.isEmpty()) {
        QMessageBox::warning(NULL, "语音合成", "无法获取访问令牌");
        return false;
    }

    QString encodedText = QUrl::toPercentEncoding(text);
    QString ttsUrl = QString(baiduTtsUrl).arg(encodedText).arg("LAPTOP-71LN9B3Q").arg(token);

    Http httputil;
    QByteArray replyData;
    bool success = httputil.getSyn(ttsUrl, replyData);
    if (success) {
        QFile file(outputFile);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(replyData);
            file.close();
            return true;
        } else {
            QMessageBox::warning(NULL, "语音合成", "无法保存音频文件");
        }
    } else {
        QMessageBox::warning(NULL, "语音合成", "语音合成失败");
    }
    return false;
}

bool Speech::playAudio(const QString &filePath)
{
    if (playerProcess) {
        playerProcess->kill();
        playerProcess->deleteLater();
        playerProcess = nullptr;
    }

    playerProcess = new QProcess();
#ifdef Q_OS_WIN
    playerProcess->start("powershell", QStringList() << "-Command" << QString("(New-Object Media.SoundPlayer '%1').PlaySync()").arg(filePath));
#else
    playerProcess->start("aplay", QStringList() << filePath);
#endif

    if (!playerProcess->waitForStarted()) {
        delete playerProcess;
        playerProcess = nullptr;
        QMessageBox::warning(NULL, "播放失败", "无法启动音频播放器");
        return false;
    }

    playerProcess->waitForFinished(-1);
    delete playerProcess;
    playerProcess = nullptr;
    return true;
}

QString Speech::getJsonValue(QByteArray ba, QString key)
{
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(ba, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
        if (jsonDocument.isObject()) {
            QJsonObject jsonObject = jsonDocument.object();
            if (jsonObject.contains(key)) {
                QJsonValue value = jsonObject.value(key);
                if (value.isString()) {
                    return value.toString();
                } else if (value.isArray()) {
                    QJsonArray arr = value.toArray();
                    QJsonValue jv = arr.at(0);
                    return jv.toString();
                }
            }
        }
    }
    return "";
}
