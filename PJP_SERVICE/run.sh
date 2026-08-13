#!/bin/bash
# run.sh - Запуск PJP Service

cd "$(dirname "$0")"

echo "🚀 Запуск PJP Service на порту 8002..."

# Активируем виртуальное окружение
if [ -d "venv" ]; then
    source venv/bin/activate
else
    echo "❌ Виртуальное окружение не найдено. Запустите ./install.sh"
    exit 1
fi

# Проверка переменных окружения
if [ ! -f .env ]; then
    echo "⚠️  .env файл не найден. Использую значения по умолчанию"
fi

# Запуск сервиса
uvicorn app.main:app --reload --port 8002 --log-level info
