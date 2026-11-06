#!/bin/bash
# stop_all.sh — Остановка всех микросервисов
#
# Использование:
#   ./stop_all.sh [--bus-only]

BUS_ONLY=false
for arg in "$@"; do
    case $arg in
        --bus-only) BUS_ONLY=true ;;
        -h|--help)
            echo "Usage: ./stop_all.sh [--bus-only]"
            exit 0 ;;
    esac
done

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"

if [ "$BUS_ONLY" = true ]; then
    echo "🛑 Остановка только BUS..."
    for pid_file in "$LOG_DIR"/BUS_*.pid "$LOG_DIR"/BUS_service.pid; do
        [ -f "$pid_file" ] || continue
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            kill -TERM $pid 2>/dev/null || kill -KILL $pid 2>/dev/null
            echo "   Остановлен процесс $pid ($(basename "$pid_file"))"
        fi
        rm -f "$pid_file"
    done
    pkill -f "uvicorn.*BUS_service" 2>/dev/null || true
    echo "✅ BUS остановлены"
    exit 0
fi

echo "🛑 Остановка всех сервисов..."

if [ -d "$LOG_DIR" ]; then
    for pid_file in "$LOG_DIR"/*.pid; do
        [ -f "$pid_file" ] || continue
        pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            kill -TERM $pid 2>/dev/null || kill -KILL $pid 2>/dev/null
            echo "   Остановлен процесс $pid ($(basename "$pid_file" .pid))"
        fi
        rm -f "$pid_file"
    done
fi

pkill -f "uvicorn.*--port" 2>/dev/null || true

echo "✅ Все сервисы остановлены"
