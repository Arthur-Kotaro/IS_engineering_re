#!/bin/bash
# stop_all.sh — Остановка всех микросервисов

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"

echo "🛑 Остановка всех сервисов..."

# Останавливаем по PID файлам
if [ -d "$LOG_DIR" ]; then
    for pid_file in "$LOG_DIR"/*.pid; do
        if [ -f "$pid_file" ]; then
            pid=$(cat "$pid_file")
            if ps -p $pid > /dev/null 2>&1; then
                kill -TERM $pid 2>/dev/null || kill -KILL $pid 2>/dev/null
                echo "   Остановлен процесс $pid"
            fi
            rm -f "$pid_file"
        fi
    done
fi

# Убиваем все uvicorn процессы
pkill -f "uvicorn.*--port" 2>/dev/null || true

echo "✅ Все сервисы остановлены"
