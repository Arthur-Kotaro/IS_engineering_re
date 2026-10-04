#!/bin/bash
cd "$(dirname "$0")"
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi
source venv/bin/activate
if ! python -c "import fastapi, sqlalchemy, redis, jwt" 2>/dev/null; then
    pip install --upgrade pip
    pip install -r requirements.txt
fi
echo "🚀 Starting Delegation Service on port 8011..."
exec uvicorn app.main:app --host 0.0.0.0 --port 8011 --reload
