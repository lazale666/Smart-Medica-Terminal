@echo off
title AI Chat Server

:RESTART
cls
echo ========================================
echo   AI Chat Server - Start Script
echo ========================================
echo.

cd /d "%~dp0"

echo [1/3] Checking Python...
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not installed or not in PATH
    echo.
    echo Please install Python and add to system environment variables
    pause
    exit /b 1
)
python --version
echo [OK] Python is ready

echo.
echo [2/3] Checking dependencies...
pip show langchain-ollama >nul 2>&1
if errorlevel 1 (
    echo [INFO] Installing dependencies...
    pip install -r requirements.txt
    if errorlevel 1 (
        echo [ERROR] Failed to install dependencies
        pause
        exit /b 1
    )
)
echo [OK] Dependencies are ready

echo.
echo [3/3] Checking Ollama...
ollama --version >nul 2>&1
if errorlevel 1 (
    echo [WARNING] Ollama not installed, please install first
    echo Download: https://ollama.ai/
    pause
    exit /b 1
)
ollama --version
echo [OK] Ollama is ready

echo.
echo ========================================
echo   Starting Server...
echo ========================================
echo.

python server.py

echo.
echo ========================================
echo   Server Stopped
echo ========================================
echo.
echo [INFO] Server will restart in 3 seconds...
echo        Press Ctrl+C to stop completely
timeout /t 3 /nobreak >nul

echo.
echo ========================================
echo   Restarting Server...
echo ========================================
goto RESTART