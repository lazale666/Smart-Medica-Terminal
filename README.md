# Smart-Medica Terminal

基于 `Python + Ollama + Qt` 的医疗咨询演示项目，包含：

- 服务端：`Server_python/server.py`
- 普通用户客户端：`Client_Qt/Client`
- 医生客户端：`Client_Qt/Client_Doctor`

当前仓库更接近“可运行的课程项目 / 演示系统”，已经具备 AI 问诊、医生会话、病历记录、会员限制、语音输入等功能，但仍存在本地账号、共享文件路径、测试不足等工程化限制。

## 项目结构

```text
Smart-Medica-Terminal/
├─ Server_python/
│  ├─ server.py                  # TCP 服务端，负责 AI 调用、医生在线管理、消息转发
│  ├─ requirements.txt           # Python 依赖
│  ├─ smart_medica.db            # SQLite 数据库
│  ├─ client_test.py             # 简单协议测试脚本
│  └─ 启动服务器.bat             # Windows 启动脚本
├─ Client_Qt/
│  ├─ Client/                    # 普通用户客户端
│  │  ├─ dontknow.pro
│  │  ├─ main.cpp
│  │  ├─ loginwidget.*           # 本地登录
│  │  ├─ menuwidget.*            # 主菜单
│  │  ├─ widget.*                # AI 问诊聊天
│  │  ├─ medicalrecordwidget.*   # 病历记录
│  │  ├─ doctorlistwidget.*      # 在线医生列表
│  │  ├─ doctordialog.*          # 医患实时对话
│  │  ├─ memberrechargewidget.*  # 会员开通
│  │  ├─ settingswidget.*        # 设置
│  │  ├─ audio.* / speech.*      # 录音与语音识别
│  │  └─ recorddetailwidget.*    # 病历详情
│  ├─ Client_Doctor/             # 医生客户端
│  │  ├─ medical.pro
│  │  ├─ main.cpp
│  │  ├─ loginwidget.*           # 本地登录
│  │  ├─ doctorchatwidget.*      # 医生侧会话中心
│  │  ├─ historydialog.*         # 历史记录查看
│  │  └─ settingswidget_doc.*    # 医生侧设置
│  ├─ photo/                     # 图片资源
│  └─ 构建说明.md
├─ 快速开始.md
├─ 技术文档.md
├─ 配置说明_BAIDU_VOICE.md
├─ 完成检查清单.md
└─ 项目完成总结.md
```

## 当前功能

### 服务端

- TCP 长连接通信，4 字节大端长度头 + JSON 载荷
- 调用 Ollama 模型处理 AI 问诊消息
- 管理在线医生列表
- 支持患者与医生之间的消息转发
- 使用 SQLite 初始化 `users / sessions / messages` 三张表
- 支持医生登录、账号自动注册、心跳响应

### 普通用户客户端

- 本地账号注册 / 登录，数据保存在 `users.json`
- AI 问诊聊天
- 聊天历史保存与切换
- 本地病历记录新增、查看、删除
- 在线医生列表与指定医生连接
- 会员开通与会员权限控制
- 语音录音输入
- 朗读设置与 `QTextToSpeech` 朗读
- 主题模式、字体颜色、浅色 / 深色样式切换

### 医生客户端

- 本地账号注册 / 登录，数据保存在 `doctors.json`
- 连接服务端并以 `doctor` 角色登录
- 会话列表管理
- 与多个患者分别对话
- 医患聊天历史保存
- 查看患者病历文件
- 医生侧服务器配置与缓存清理

## 真实运行行为

以下几点很重要，和源码保持一致：

1. 普通用户登录不是服务端鉴权，而是本地 `users.json` 鉴权。
2. 医生先经过本地 `doctors.json` 鉴权，然后医生客户端再向服务端发送 `login` 消息。
3. 普通用户 AI 问诊直接向服务端发送 `message`，服务端调用 Ollama 返回 `ai_response`。
4. 患者端“名医对话”不是自动分诊 UI，而是先请求在线医生列表，再选择一个医生建立连接。
5. 非会员每天默认最多免费问诊 `5` 次，计数保存在 `QSettings`。
6. 病历和聊天记录主要保存在本地文件系统，不在服务端集中存储。

## 数据落盘位置

- 普通用户本地账号：客户端工作目录下的 `users.json`
- 医生本地账号：医生客户端工作目录下的 `doctors.json`
- 服务端数据库：`Server_python/smart_medica.db`
- 普通用户聊天历史：应用目录下 `chat_history/<用户名>/chat_*.txt`
- 医生聊天历史：应用目录下 `chat_history_doctor/<医生名>/doctor_chat_*.txt`
- 病历记录：`%USERPROFILE%/SmartMedica/records/<用户名>/record_*.txt`
- 会员状态、免费次数、主题、服务器配置：`QSettings`

## 环境要求

### 服务端

- Python 3.8+
- `pip`
- Ollama
- 已下载可用模型，默认是 `qwen2.5:7b`

### Qt 客户端

- Qt 6.x
- MinGW 64-bit
- 普通用户端需要模块：`Widgets`、`Network`、`Multimedia`、`TextToSpeech`
- 医生端需要模块：`Widgets`、`Network`、`Multimedia`

## 快速启动

### 1. 启动服务端

```powershell
cd .\Server_python
pip install -r requirements.txt
ollama pull qwen2.5:7b
python server.py
```

也可以直接双击：

```text
Server_python/启动服务器.bat
```

### 2. 构建普通用户客户端

```powershell
cd .\Client_Qt\Client
qmake dontknow.pro
mingw32-make
```

### 3. 构建医生客户端

```powershell
cd .\Client_Qt\Client_Doctor
qmake medical.pro
mingw32-make
```

详细构建步骤见 [Client_Qt/构建说明.md](Client_Qt/构建说明.md)。

## 文档索引

- [快速开始.md](快速开始.md)
- [技术文档.md](技术文档.md)
- [配置说明_BAIDU_VOICE.md](配置说明_BAIDU_VOICE.md)
- [完成检查清单.md](完成检查清单.md)
- [项目完成总结.md](项目完成总结.md)
- [Client_Qt/构建说明.md](Client_Qt/构建说明.md)

## 已知限制

1. 账号体系分裂。
   普通用户和医生首先依赖本地 JSON 文件登录，和服务端 SQLite 账号体系并不统一。

2. 病历查看依赖本地共享目录。
   医生端读取的是本机 `%USERPROFILE%/SmartMedica/records/...`。如果患者和医生运行在不同机器，医生端默认看不到患者病历。

3. 服务端虽然建了 `sessions`、`messages` 表，但当前代码没有把会话与消息完整写入数据库。

4. 百度语音配置目前写死在源码中，不适合生产环境。

5. 仓库中没有自动化测试、打包脚本和统一部署方案。

## 适用范围

本项目适合作为：

- Qt 网络课程项目
- AI + 桌面端集成演示
- TCP 自定义协议示例
- 小型医疗咨询交互原型

如果要继续工程化，建议优先处理统一鉴权、集中存储、日志、测试和部署。
