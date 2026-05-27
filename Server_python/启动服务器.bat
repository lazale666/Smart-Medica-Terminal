@echo off
echo ========================================
echo   AI Chat Server - 启动脚本
echo ========================================
echo.

cd /d "%~dp0"

echo [1/3] 检查Python环境...
python --version >nul 2>&1
if errorlevel 1 (
    echo [错误] Python未安装或未添加到PATH
    pause
    exit /b 1
)
echo [OK] Python环境正常

echo.
echo [2/3] 检查依赖...
pip show langchain-ollama >nul 2>&1
if errorlevel 1 (
    echo [提示] 正在安装依赖...
    pip install -r requirements.txt
)
echo [OK] 依赖检查完成

echo.
echo [3/3] 检查Ollama...
ollama --version >nul 2>&1
if errorlevel 1 (
    echo [警告] Ollama未安装，请先安装Ollama
    echo 下载地址: https://ollama.ai/
    pause
    exit /b 1
)

echo [OK] Ollama已安装

echo.
echo ========================================
echo   正在启动服务器...
echo ========================================
echo.

python server.py

pause
