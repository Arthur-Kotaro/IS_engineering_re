#!/bin/bash
# BUS_service/run.sh — Запуск BUS Service
#
# Использование:
#   ./run.sh [--port PORT] [--group GROUP]
#
# По умолчанию: --port 8005 --group default

cd "$(dirname "$0")"

PORT=8005
GROUP=default

while [[ $# -gt 0 ]]; do
    case $1 in
        --port)
            PORT="$2"; shift 2 ;;
        --group)
            GROUP="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: ./run.sh [--port PORT] [--group GROUP]"
            exit 0 ;;
        *)
            echo "Unknown argument: $1"
            exit 1 ;;
    esac
done

export BUS_GROUP="$GROUP"

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

source venv/bin/activate

if ! python -c "import fastapi, sqlalchemy" 2>/dev/null; then
    pip install --upgrade pip
    pip install -r requirements.txt
fi

echo "🚀 Starting BUS Service (group=$GROUP) on port $PORT..."
exec uvicorn app.main:app --host 0.0.0.0 --port $PORT --reload
