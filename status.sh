#!/bin/bash
# status.sh — Проверка статуса всех сервисов

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "📊 Статус сервисов:"
echo "==================="

check_service() {
    local name=$1
    local port=$2
    
    if curl -s -o /dev/null -w "%{http_code}" "http://localhost:$port/health" 2>/dev/null | grep -q "200"; then
        echo -e "${GREEN}✅ $name (:$port) - RUNNING${NC}"
    else
        echo -e "${RED}❌ $name (:$port) - STOPPED${NC}"
    fi
}

check_service "User Service" 8000
check_service "Project Service" 8001
check_service "PJP Service" 8002
check_service "MG Service" 8003
check_service "PROTO Service" 8004
check_service "Navigation Service" 8009
check_service "API Gateway" 8080

echo ""
echo "🔍 Процессы uvicorn:"
ps aux | grep "uvicorn.*--port" | grep -v grep || echo "Нет запущенных uvicorn процессов"
