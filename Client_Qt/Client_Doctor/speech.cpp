#include "speech.h"

Speech::Speech() {}

QString Speech::speechIdentify(QString fileName)
{
    QString tokenUrl = QString(baiduTokenUrl).arg(client_id).arg(client_secret);
    QMap<QString,QString> headers;
    headers.insert(QString("Content-Type"),QString("audio/pcm;rate=16000"));
    QByteArray requestData;
    QByteArray replyData;
    Http httputil;
    bool success = httputil.postSyn(tokenUrl,headers,requestData,replyData);
    if(success){
        QString key = "access_token";
        accessToken = getJsonVale(replyData,key);
        qDebug()<<accessToken;
    }else{
        return "";
    }
    QString baiduSpeech = QString(baiduSpeechurl).arg("LAPTOP-71LN9B3Q").arg(accessToken);
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);
    requestData = file.readAll();
    file.close();
    replyData.clear();

    bool result = httputil.postSyn(baiduSpeech,headers,requestData,replyData);
    if(result)
    {
        QString key = "result";
        QString text = getJsonVale(replyData,key);
        return text;
    }else{
        QMessageBox::warning(NULL,"识别提示","识别失败");
    }
    return "";
}
QString Speech::getJsonVale(QByteArray ba,QString key)
{
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(ba,&parseError);
    if(parseError.error == QJsonParseError::NoError)
    {
        if(jsonDocument.isObject())
        {
            QJsonObject jsonObject = jsonDocument.object();
            if(jsonObject.contains(key))
            {
                QJsonValue value = jsonObject.value(key);
                if(value.isString())
                {
                    return value.toString();
                }else if(value.isArray()){
                    QJsonArray arr = value.toArray();
                    QJsonValue jv = arr.at(0);
                    return jv.toString();
                }
            }
        }
    }
}
