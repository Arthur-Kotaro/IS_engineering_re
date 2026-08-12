#!/bin/bash
# run.sh — Запуск Navigation Service

cd "$(dirname "$0")"

if [ -d "venv" ]; then
    source venv/bin/activate
fi

echo "🚀 Starting Navigation Service on port 8009..."
uvicorn app.main:app --reload --port 8009 --log-level info
