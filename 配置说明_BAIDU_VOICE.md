# 百度语音配置说明

本文档只说明当前仓库里“百度语音相关代码”的真实用法。

## 1. 当前项目中百度语音的作用

普通用户客户端里，百度语音目前用于：

- 录音文件转文字

对应代码：

- `Client_Qt/Client/audio.cpp`
- `Client_Qt/Client/speech.cpp`
- `Client_Qt/Client/speech.h`

需要特别注意：

1. “朗读 AI 回复”不是走百度语音。
   当前朗读使用的是 Qt 自带的 `QTextToSpeech`。

2. 百度 TTS 的代码虽然存在于 `Speech` 类里，但普通聊天界面当前没有直接调用这条路径。

## 2. 当前配置位置

百度语音相关常量写在：

```text
Client_Qt/Client/speech.h
```

当前可见内容包括：

- `client_id`
- `client_secret`
- `baiduTokenUrl`
- `baiduSpeechUrl`
- `baiduTtsUrl`

另外，调用时还写死了一个 `cuid`，位置在：

```text
Client_Qt/Client/speech.cpp
```

## 3. 推荐做法

如果你要继续使用百度语音，请至少替换以下内容：

1. `client_id`
2. `client_secret`
3. `speech.cpp` 里构造请求时使用的 `cuid`

建议把它们改成你自己的配置，不要继续使用仓库里已有值。

## 4. 修改步骤

### 步骤 1：申请百度智能云语音服务

需要准备：

- 百度智能云账号
- 语音识别应用
- 对应的 API Key / Secret Key

### 步骤 2：修改 `speech.h`

把以下常量替换成你自己的值：

```cpp
const QString client_id = "...";
const QString client_secret = "...";
```

### 步骤 3：修改 `speech.cpp`

将请求里使用的 `cuid` 替换成你自己的稳定标识，例如：

```cpp
QString baiduSpeech = QString(baiduSpeechUrl).arg("SmartMedicaClient").arg(token);
```

如果你后续也要启用百度 TTS，同样需要替换 `textToSpeech()` 里的 `cuid`。

## 5. 运行前提

百度语音识别要正常工作，需要同时满足：

- 录音设备可用
- Qt Multimedia 模块已安装
- 客户端能访问公网
- 百度接口凭证有效
- 录音文件格式满足接口要求

当前代码发送的请求头是：

```text
Content-Type: audio/pcm;rate=16000
```

因此如果你修改了录音参数，需要同步检查百度接口参数是否仍然匹配。

## 6. 当前实现限制

1. 密钥硬编码在源码中，不安全。
2. 使用 `http` 而不是更稳妥的密钥外置方案。
3. 失败处理较简单，主要通过弹窗提示。
4. 没有做请求重试、超时兜底和额度监控。

## 7. 更合理的改造建议

建议按下面顺序重构：

1. 把百度密钥迁移到本地配置文件或环境变量。
2. 为 `Speech` 增加明确的配置加载逻辑。
3. 将 `cuid` 改成可配置值。
4. 为语音识别失败增加更清晰的错误日志。
5. 如果最终只保留 Qt 朗读，可以把百度 TTS 相关代码移除，避免双通道维护。

## 8. 常见问题

### 录音后没有识别结果

优先检查：

- 百度密钥是否有效
- 网络是否可访问百度接口
- 录音文件是否成功生成 `record.wav`
- 录音格式是否与接口头一致

### 可以录音，但朗读按钮无声音

这通常不是百度问题，而是 `QTextToSpeech` 环境问题。请检查：

- Qt `TextToSpeech` 模块是否安装
- Windows 语音引擎是否可用
- 设置页中的朗读音量是否过低

### 医生端为什么没有同样的百度配置

因为当前医生端没有把百度语音识别接入到核心流程里。仓库中的主要语音入口仍然在普通用户端。
