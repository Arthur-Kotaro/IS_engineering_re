#!/bin/bash
# run.sh — Запуск Mastergraphics Service (Stub)

cd "$(dirname "$0")"

if [ -d "venv" ]; then
    source venv/bin/activate
fi

echo "🚀 Starting Mastergraphics Service on port 8003..."
uvicorn app.main:app --reload --port 8003 --log-level info
