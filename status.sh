#!/bin/bash
# status.sh — Проверка статуса всех сервисов

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

echo -e "${BLUE}📊 Статус сервисов:${NC}"
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

# --- Инфраструктура ---
check_service "User Service"         8000
check_service "Project Service"      8001
check_service "Navigation Service"   8009
check_service "Auth Service"         8010
check_service "Delegation Service"   8011
check_service "Notification Service" 8012
check_service "UI Composer Service"  8020

# --- Бизнес-сервисы ---
echo -e "${YELLOW}--- Бизнес-сервисы ---${NC}"
check_service "PJP Service"          8002
check_service "MG Service"           8003
check_service "PROTO Service"        8004

# --- Gateway (nginx) ---
echo -e "${YELLOW}--- Gateway ---${NC}"
if systemctl is-active --quiet nginx 2>/dev/null; then
    echo -e "${GREEN}✅ Nginx Gateway (systemd) - RUNNING${NC}"
else
    echo -e "${RED}❌ Nginx Gateway (systemd) - STOPPED${NC}"
fi
# Дополнительно проверим, что 8080 реально отвечает
check_service "Gateway (HTTP)"       8080

echo ""
echo -e "${BLUE}🔍 Процессы uvicorn:${NC}"
ps aux | grep "uvicorn.*--port" | grep -v grep || echo "Нет запущенных uvicorn процессов"

echo ""
echo -e "${BLUE}🔍 PID-файлы:${NC}"
if ls "$PROJECT_ROOT/logs"/*.pid >/dev/null 2>&1; then
    for pid_file in "$PROJECT_ROOT/logs"/*.pid; do
        [ -f "$pid_file" ] || continue
        local_name=$(basename "$pid_file" .pid)
        local_pid=$(cat "$pid_file")
        if ps -p "$local_pid" > /dev/null 2>&1; then
            echo -e "${GREEN}  ✅ $local_name (PID: $local_pid)${NC}"
        else
            echo -e "${RED}  ❌ $local_name (PID: $local_pid — мёртвый)${NC}"
        fi
    done
else
    echo "  PID-файлов нет"
fi
