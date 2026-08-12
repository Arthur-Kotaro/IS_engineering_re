#!/bin/bash
# run.sh — Запуск PROTO Service

cd "$(dirname "$0")"

if [ -d "venv" ]; then
    source venv/bin/activate
fi

echo "🚀 Starting PROTO Service on port 8004..."
uvicorn app.main:app --reload --port 8004 --log-level info
