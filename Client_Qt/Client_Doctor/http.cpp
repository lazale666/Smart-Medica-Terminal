#include "http.h"

Http::Http() {}

bool Http::postSyn(QString url,QMap<QString,QString>headerdata,QByteArray requestData,QByteArray &replayData)
{
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(url);
    QMapIterator<QString,QString> it(headerdata);
    while(it.hasNext())
    {
        it.next();
        request.setRawHeader(it.key().toLatin1(),it.value().toLatin1());
    }
    QNetworkReply *replay = manager.post(request,requestData);
    QEventLoop l;
    connect(replay,&QNetworkReply::finished,&l,&QEventLoop::quit);
    l.exec();
    if(replay!=nullptr&&replay->error()==QNetworkReply::NoError){
        replayData = replay->readAll();
        qDebug()<<replayData;
        return true;
    }else{
        qDebug()<<"请求失败";
        return false;
    }
}
