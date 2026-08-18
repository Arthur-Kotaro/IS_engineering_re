#!/bin/bash
# run.sh — Запуск Notification Service

cd "$(dirname "$0")"

if [ ! -d "venv" ]; then
    echo "Creating virtual environment for Notification Service..."
    python3 -m venv venv
    source venv/bin/activate
    pip install --upgrade pip
    pip install -r requirements.txt
else
    source venv/bin/activate
fi

if ! python -c "import fastapi" 2>/dev/null; then
    echo "Installing dependencies..."
    pip install -r requirements.txt
fi

echo "🚀 Starting Notification Service on port 8012..."
exec uvicorn app.main:app --host 0.0.0.0 --port 8012 --reload
