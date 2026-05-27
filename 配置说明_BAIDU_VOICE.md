# 百度语音 API 配置指南

## 1. 获取百度语音 API 凭证

### 步骤 1：注册百度智能云账号

1. 访问百度智能云官网：https://cloud.baidu.com/
2. 注册并登录账号

### 步骤 2：开通语音技术服务

1. 登录后，进入"产品服务" -> "人工智能" -> "语音技术"
2. 点击"立即使用"或"开通服务"
3. 完成实名认证（如果需要）

### 步骤 3：创建应用获取凭证

1. 进入"控制台" -> "语音技术"
2. 点击"创建应用"
3. 填写应用信息（应用名称、应用描述等）
4. 在"API Key"和"Secret Key"处获取你的凭证
5. **重要**：确保开通了以下服务：
   - ✅ 语音识别（ASR）
   - ✅ 语音合成（TTS）- 可选

## 2. 配置凭证到代码

找到 `speech.h` 文件，修改以下内容：

```cpp
// 文件路径：Client_Qt/Client/speech.h

// 修改前（第14-16行）：
const QString client_id = "";
const QString client_secret = "";

// 修改后：
const QString client_id = "你的API_KEY";      // 例如：aBcDeFgHiJkLmNoPqRsTuVwXyZ
const QString client_secret = "你的SECRET_KEY";  // 例如：AbCdEfGhIjKlMnOpQrStUvWxYz0123456789
```

### 完整示例

```cpp
const QString baiduTokenUrl = "http://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=%1&client_secret=%2&";
const QString client_id = "LKF8XXXXXXXXXXXXX";  // 替换为你的 API Key
const QString client_secret = "hXXXXXXXXXXXXXXXXXXXXXXXXX";  // 替换为你的 Secret Key
const QString baiduSpeechurl = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=%1&token=%2";
```

## 3. 验证配置

### 测试语音识别

1. 重新编译项目
2. 运行客户端
3. 连接服务器
4. 按住麦克风按钮说话
5. 松开按钮，等待识别结果

### 预期输出

```
开始录音...
录音结束，正在识别...
识别结果：你好
我：你好
茯苓：你好！有什么可以帮助你的吗？
```

## 4. 常见问题

### 问题 1：识别失败

**可能原因**：
- API Key 或 Secret Key 配置错误
- 网络连接问题
- 音频文件格式不支持
- API 服务未开通

**解决方案**：
1. 检查 `speech.h` 中的凭证是否正确
2. 确保网络可以访问百度 API
3. 检查 `audio.cpp` 中的音频格式是否为 PCM 16000Hz
4. 登录百度智能云控制台，确认语音识别服务已开通

### 问题 2：API 调用频率限制

**说明**：
百度语音 API 有每日调用次数限制（免费额度）

**解决方案**：
- 升级到付费套餐
- 合理控制调用频率

### 问题 3：录音文件不存在

**可能原因**：
- 录音未成功启动
- 录音时长太短
- 文件保存路径错误

**解决方案**：
1. 检查是否有麦克风权限
2. 确保录音时长超过 1 秒
3. 检查音频文件是否生成在正确路径

## 5. 音频格式要求

百度语音识别 API 对音频格式的要求：

| 参数 | 要求 |
|------|------|
| 采样率 | 16000 Hz |
| 声道 | 单声道 |
| 位深 | 16bit |
| 编码 | PCM |
| 格式 | wav 或 pcm |

你的 `audio.cpp` 中已经配置了正确的格式：

```cpp
QAudioFormat format;
format.setSampleRate(16000);    // ✅ 采样率
format.setChannelCount(1);      // ✅ 单声道
format.setSampleFormat(QAudioFormat::Int16);  // ✅ 16bit
```

## 6. 获取帮助

如果遇到其他问题，可以：

1. 查看 Qt Creator 的"应用程序输出"面板的错误信息
2. 检查服务器端的日志输出
3. 参考百度语音技术文档：https://cloud.baidu.com/doc/SPEECH/index.html

## 7. 安全提示

⚠️ **重要**：
- 不要将 API Key 和 Secret Key 提交到公开的代码仓库
- 建议使用环境变量或配置文件来管理敏感信息
- 定期更换密钥
