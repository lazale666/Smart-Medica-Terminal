#include "audio.h"

Audio::Audio(QObject *parent)
    : QObject{parent}
{}
QAudioFormat Audio::getNearestFormat(const QAudioDevice &device, const QAudioFormat &targetFormat)
{
    if (device.isFormatSupported(targetFormat)) {
        return targetFormat;
    }

    QAudioFormat nearestFormat = device.preferredFormat();

    // 对齐采样率
    if (device.isFormatSupported(
            [&nearestFormat, &targetFormat]() -> QAudioFormat {
                auto fmt = nearestFormat;
                fmt.setSampleRate(targetFormat.sampleRate());
                return fmt;
            }()
            )) {
        nearestFormat.setSampleRate(targetFormat.sampleRate());
    }

    // 对齐声道数
    if (device.isFormatSupported(
            [&nearestFormat, &targetFormat]() -> QAudioFormat {
                auto fmt = nearestFormat;
                fmt.setChannelCount(targetFormat.channelCount());
                return fmt;
            }()
            )) {
        nearestFormat.setChannelCount(targetFormat.channelCount());
    }

    // 对齐采样格式
    if (device.isFormatSupported(
            [&nearestFormat, &targetFormat]() -> QAudioFormat {
                auto fmt = nearestFormat;
                fmt.setSampleFormat(targetFormat.sampleFormat());
                return fmt;
            }()
            )) {
        nearestFormat.setSampleFormat(targetFormat.sampleFormat());
    }
    return nearestFormat;
}

// 音频录制初始化函数（替代你原有的Qt5代码）
void Audio::startAudioRecord(const QString &filename)
{
    // 1. Qt6获取默认音频输入设备（替代Qt5的QAudioDeviceInfo::defaultInputDevice()）
    QAudioDevice audioDevice = QMediaDevices::defaultAudioInput();
    if(!audioDevice.isNull())
    {
        m_file = new QFile(this); // 加上父对象，避免内存泄漏
        m_file->setFileName(filename);
        // 打开文件（保留原有逻辑）
        if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qCritical() << "无法打开录制文件：" << filename;
            delete m_file;
            m_file = nullptr;
            return;
        }

        // 2. Qt6配置QAudioFormat（适配6.10无setSampleSize/setCodec）
        QAudioFormat format;
        format.setSampleRate(16000);    // 采样率（和Qt5一致）
        format.setChannelCount(1);      // 声道数（和Qt5一致）
        format.setSampleFormat(QAudioFormat::Int16); // 替代Qt5的setSampleSize(16)
        // Qt6.10移除setCodec，默认就是PCM，无需设置

        // 3. 检查格式是否支持（替代Qt5的isFormatSupported）
        if(!audioDevice.isFormatSupported(format))
        {
            // 替代Qt5的nearestFormat
            format = getNearestFormat(audioDevice, format);
        }

        // 4. Qt6构造QAudioInput：必须先传设备，再传格式（核心变更）
        m_audioInput = new QAudioSource(audioDevice, format, this);
        // 开始录制（逻辑和Qt5一致）
        m_audioInput->start(m_file);

        qDebug() << "开始录制音频，使用格式：采样率=" << format.sampleRate() << "，声道数=" << format.channelCount();
    } else {
        QMessageBox::information(nullptr, tr("Record"), tr("Current No Record Device"));
    }
}

// 停止录制的辅助函数（可选，补充完整功能）
void Audio::stopAudioRecord()
{
    if (m_audioInput) {
        m_audioInput->stop();
        delete m_audioInput;
        m_audioInput = nullptr;
    }
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

Audio::~Audio()
{

}
