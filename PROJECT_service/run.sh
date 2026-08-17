#!/bin/bash
# run.sh — Запуск Project Service

cd "$(dirname "$0")"

# Проверяем виртуальное окружение
if [ ! -d "venv" ]; then
    echo "Creating virtual environment for Project Service..."
    python3 -m venv venv
    source venv/bin/activate
    pip install --upgrade pip
    pip install fastapi uvicorn sqlalchemy asyncpg httpx python-dotenv pydantic-settings
else
    source venv/bin/activate
fi

# Проверяем, установлены ли зависимости
if ! python -c "import fastapi" 2>/dev/null; then
    echo "Installing dependencies for Project Service..."
    pip install fastapi uvicorn sqlalchemy asyncpg httpx python-dotenv pydantic-settings
fi

echo "🚀 Starting Project Service on port 8001..."
exec uvicorn app.main:app --host 0.0.0.0 --port 8001 --reload
