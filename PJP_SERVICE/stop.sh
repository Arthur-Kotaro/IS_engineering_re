#!/bin/bash
# stop.sh - Остановка PJP Service

echo "🛑 Остановка PJP Service..."

# Находим процесс uvicorn
PID=$(ps aux | grep "uvicorn app.main:app" | grep -v grep | awk '{print $2}')

if [ -n "$PID" ]; then
    kill -9 $PID
    echo "✅ Сервис остановлен (PID: $PID)"
else
    echo "⚠️  Сервис не запущен"
fi
