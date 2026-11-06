#!/bin/bash
# restart_all.sh — Перезапуск всех сервисов
#
# Все аргументы передаются в start_all.sh:
#   ./restart_all.sh -b
#   ./restart_all.sh -b --bus-multi
#   ./restart_all.sh -f --bus-multi

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT" || exit 1

for script in stop_all.sh start_all.sh; do
    if [ ! -x "$PROJECT_ROOT/$script" ]; then
        echo -e "${RED}❌ $script не найден${NC}"
        exit 1
    fi
done

echo -e "${BLUE}🔄 Перезапуск микросервисов${NC}"

echo -e "\n${YELLOW}▶ Остановка...${NC}"
"$PROJECT_ROOT/stop_all.sh"

sleep 2

PORTS=(8000 8001 8002 8003 8004 8005 8009 8010 8011 8012 8013 8014 8015 8016 8017 8018 8019 8020)
BUSY_PORTS=()
for port in "${PORTS[@]}"; do
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1; then
        BUSY_PORTS+=("$port")
    fi
done

if [ ${#BUSY_PORTS[@]} -gt 0 ]; then
    echo -e "${YELLOW}⚠  Порты всё ещё заняты: ${BUSY_PORTS[*]}${NC}"
fi

echo -e "\n${YELLOW}▶ Запуск...${NC}"
"$PROJECT_ROOT/start_all.sh" "$@"

echo -e "\n${GREEN}✅ Перезапуск завершён${NC}"
