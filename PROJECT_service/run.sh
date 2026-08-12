#!/bin/bash
# run.sh — Запуск Project Service

cd "$(dirname "$0")"

if [ -d "venv" ]; then
    source venv/bin/activate
fi

echo "🚀 Starting Project Service on port 8001..."
uvicorn app.main:app --reload --port 8001 --log-level info
