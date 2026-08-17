#!/bin/bash
# run.sh для Auth Service

cd "$(dirname "$0")"

# Проверяем, существует ли виртуальное окружение
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
    source venv/bin/activate
    pip install httpx redis pyjwt fastapi uvicorn
else
    source venv/bin/activate
fi

# Запускаем сервис
exec uvicorn main:app --host 0.0.0.0 --port 8010 --reload
