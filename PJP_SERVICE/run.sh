#!/bin/bash
# run.sh — Запуск PJP Service

cd "$(dirname "$0")"

if [ -d "venv" ]; then
    source venv/bin/activate
fi

echo "🚀 Starting PJP Service on port 8002..."
uvicorn app.main:app --reload --port 8002 --log-level info
