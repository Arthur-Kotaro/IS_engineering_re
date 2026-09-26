#!/bin/bash
# run.sh для Auth Service

cd "$(dirname "$0")"

if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

source venv/bin/activate

if ! python -c "import fastapi, jwt, redis" 2>/dev/null; then
    echo "Installing dependencies..."
    pip install --upgrade pip
    pip install -r requirements.txt
fi

echo "🚀 Starting Auth Service on port 8010..."
exec uvicorn main:app --host 0.0.0.0 --port 8010 --reload
