#!/bin/bash
cd "$(dirname "$0")"
if [ ! -d "venv" ]; then
    python3 -m venv venv
fi
source venv/bin/activate
if ! python -c "import fastapi, yaml, redis, httpx" 2>/dev/null; then
    pip install --upgrade pip
    pip install -r requirements.txt
fi
echo "🚀 Starting UI Composer Service on port 8020..."
exec uvicorn src.main:app --host 0.0.0.0 --port 8020 --reload
