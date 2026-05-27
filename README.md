# 医疗智能体 AI Chat System

## 项目简介

这是一个基于 Python 服务器和 Qt 客户端的 AI 聊天系统，集成了 Ollama AI 模型，提供语音输入输出功能。

## 项目结构

```
Project_End/
├── Server/                              # Python 服务器端
│   ├── server.py                        # 主服务器代码
│   ├── requirements.txt                 # Python 依赖
│   └── 启动服务器.bat                   # 一键启动脚本
│
├── Client_Qt/Client/                    # Qt 客户端
│   ├── main.cpp                         # 程序入口
│   ├── loginwidget.h/cpp                # 登录界面
│   ├── loginwidget.ui                   # 登录界面 UI
│   ├── widget.h/cpp                     # 主聊天界面
│   ├── widget.ui                        # 主聊天界面 UI
│   ├── dialog.h/cpp                     # 重连对话框
│   ├── dialog.ui                        # 重连对话框 UI
│   ├── audio.h/cpp                      # 音频录制功能
│   ├── mainwindow.h/cpp                 # 主窗口（预留）
│   ├── mainwindow.ui                    # 主窗口 UI
│   └── dontknow.pro                     # Qt 项目文件
│
├── README.md                            # 本文档
└── 配置说明_BAIDU_VOICE.md              # 百度语音 API 配置说明
```

## 功能特性

### 服务器端

- [x] TCP 服务器，支持多客户端连接
- [x] 集成 Ollama AI 模型（默认使用 qwen2.5:7b）
- [x] 实时日志输出，显示调试信息
- [x] 自动检测和启动 Ollama 服务
- [x] 兼容两种消息格式（`data` 和 `message`）

### 客户端

- [x] 账号密码登录
- [x] 用户注册功能
- [x] 服务器连接界面
- [x] 保存历史连接记录
- [x] 支持新建、删除连接
- [x] 自动重连机制（最多 10 次）
- [x] AI 对话界面
- [x] 对话内容实时保存和显示
- [x] 语音输入功能
- [x] 实时显示连接状态

## 安装和运行

### 1. 服务器端

#### 环境要求

- Python 3.8+
- Ollama 已安装并运行

#### 安装步骤

```bash
# 进入服务器目录
cd Project_End/Server

# 安装 Python 依赖
pip install -r requirements.txt

# 确保 Ollama 服务正在运行
ollama serve

# 拉取 AI 模型（如果尚未拉取）
ollama pull qwen2.5:7b

# 启动服务器
python server.py

# Windows 用户可直接双击：启动服务器.bat
```

服务器将在 `0.0.0.0:9999` 端口监听。

### 2. Qt 客户端

#### 环境要求

- Qt 6.x
- MinGW 64-bit 编译器
- Qt Multimedia 模块
- Qt Network 模块

#### 编译步骤

```bash
# 进入 Qt 项目目录
cd Project_End/Client_Qt/Client

# 使用 Qt Creator 打开 dontknow.pro
# 或者在命令行编译：
qmake dontknow.pro
mingw32-make

# 运行客户端
```

## 使用指南

### 1. 启动服务器

```bash
python server.py
```

服务器启动后，会显示以下日志：
- Ollama 服务状态检查
- AI 模型加载状态
- 客户端连接/断开日志
- 消息处理日志

### 2. 客户端使用

#### 登录/注册

1. 打开客户端程序
2. 在登录界面输入用户名和密码
3. 点击"登录"进入主界面
4. 或点击"注册"创建新账号

#### 连接服务器

1. 在服务器地址栏输入服务器 IP（例如：127.0.0.1）
2. 输入端口号（默认：9999）
3. 点击"连接服务器"按钮
4. 连接成功后会显示状态信息

#### AI 对话

1. 在输入框中输入消息
2. 点击"发送"按钮或按回车发送消息
3. 等待 AI（茯苓）回复
4. 对话内容会实时显示在文本区域

#### 语音输入

1. 按住麦克风按钮开始录音
2. 松开按钮结束录音
3. 录音文件会保存在本地
4. （可选）集成百度语音 API 进行语音识别

## 网络协议

### 消息格式

使用 JSON 格式，通过 TCP 发送：

#### 数据包结构

```
[4字节大端序长度][JSON数据]
```

#### 发送消息

```json
{
  "type": "message",
  "data": "用户输入的文本"
}
```

#### AI 响应

```json
{
  "type": "ai_response",
  "data": "AI 生成的响应文本"
}
```

### 兼容性说明

服务器同时兼容两种字段名：
- `data`（推荐）
- `message`（旧版本兼容）

## 修复的关键问题

### 1. 消息发送问题

**问题**：客户端发送消息时没有发送 4 字节长度头，服务器收不到消息

**修复**：
- 在 `widget.cpp` 的 `on_pushButton_clicked()` 中添加了长度头发送逻辑
- 使用 `QDataStream` 确保大端序字节序
- 添加了 `flush()` 确保数据立即发送

### 2. 消息读取问题

**问题**：客户端读取消息时没有处理粘包，字段名不匹配

**修复**：
- 在 `widget.h` 中添加了 `QByteArray buffer` 来处理粘包
- 重写了 `readData()` 函数，正确处理数据包拆分
- 修正了字段名从 `"message"` 改为 `"data"`

### 3. 服务器兼容性问题

**问题**：服务器只读取 `data` 字段，不兼容旧版本格式

**修复**：
- 在 `server.py` 中添加了对 `message` 字段的兼容读取

## 开发说明

### 添加新功能

1. 在相应的源文件中添加功能
2. 在 UI 文件中添加控件
3. 在头文件中声明函数和信号槽
4. 连接信号槽并实现功能

### 自定义 AI 模型

修改 `server.py` 中的 `model_name` 变量：
```python
model_name = "your-model-name"
```

### 扩展语音服务

可以在项目中添加百度语音 API 集成，实现语音识别和合成功能。

## 注意事项

1. **网络要求**：确保客户端和服务器之间的网络通畅
2. **Ollama 服务**：确保 Ollama 服务正常运行
3. **防火墙**：确保防火墙允许 9999 端口通信
4. **录音权限**：确保客户端有麦克风访问权限

## 常见问题

### 服务器无法启动

- 检查 Ollama 是否已安装
- 检查端口是否被占用
- 查看 Python 依赖是否完整安装

### 客户端连接失败

- 检查服务器是否运行
- 检查 IP 和端口是否正确
- 检查网络连接
- 检查防火墙设置

### 发送消息服务器收不到

- 确保已应用本项目的修复代码
- 检查服务器日志是否有数据到达
- 确认网络连接正常

### AI 无响应

- 检查 Ollama 服务是否运行
- 检查模型是否正确加载
- 查看服务器日志

## 技术栈

- **后端**：Python 3.8+, sockets, langchain-ollama
- **前端**：Qt 6.x, C++17
- **AI 引擎**：Ollama
- **音频**：Qt Multimedia

## 许可证

本项目仅供学习和参考使用。
