#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QAudioSource>
#include <QFile>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QDebug>
#include <QMessageBox>

class Audio : public QObject
{
    Q_OBJECT
public:
    explicit Audio(QObject *parent = nullptr);

    void startAudioRecord(const QString &filename);
    void stopAudioRecord();
    QAudioFormat getNearestFormat(const QAudioDevice &device, const QAudioFormat &targetFormat);

    ~Audio();

signals:
private:
    QAudioSource *m_audioInput;
    QFile *m_file;
};

#endif // AUDIO_H
