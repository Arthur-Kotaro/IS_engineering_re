#!/bin/bash
# restart_all.sh — Перезапуск всех сервисов
#
# Использование:
#   ./restart_all.sh [OPTION]
#
# Все аргументы передаются в start_all.sh. То есть:
#   ./restart_all.sh -d    перезапуск в режиме default (инфраструктура)
#   ./restart_all.sh -b    перезапуск в режиме business
#   ./restart_all.sh -f    перезапуск в режиме full
#   ./restart_all.sh       то же, что -d (default)
#
# Справка по режимам:
#   ./start_all.sh -h

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT" || {
    echo -e "${RED}❌ Не удалось перейти в $PROJECT_ROOT${NC}"
    exit 1
}

# --- Проверка, что нужные скрипты на месте ---
for script in stop_all.sh start_all.sh; do
    if [ ! -x "$PROJECT_ROOT/$script" ]; then
        echo -e "${RED}❌ Скрипт $script не найден или не исполняемый${NC}"
        echo -e "${YELLOW}   Проверь: ls -la $PROJECT_ROOT/$script${NC}"
        exit 1
    fi
done

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🔄 Перезапуск микросервисов${NC}"
echo -e "${BLUE}========================================${NC}"

# --- Стоп ---
echo -e "\n${YELLOW}▶ Остановка...${NC}"
"$PROJECT_ROOT/stop_all.sh"
STOP_RC=$?

if [ $STOP_RC -ne 0 ]; then
    echo -e "${YELLOW}⚠  stop_all.sh завершился с кодом $STOP_RC, продолжаем всё равно${NC}"
fi

# --- Пауза, чтобы порты успели освободиться ---
sleep 2

# --- Проверка, что порты реально освободились ---
PORTS=(8000 8001 8002 8003 8004 8009 8010 8011 8012 8020)
BUSY_PORTS=()
for port in "${PORTS[@]}"; do
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1; then
        BUSY_PORTS+=("$port")
    fi
done

if [ ${#BUSY_PORTS[@]} -gt 0 ]; then
    echo -e "${YELLOW}⚠  Порты всё ещё заняты: ${BUSY_PORTS[*]}${NC}"
    echo -e "${YELLOW}   Возможно, остались процессы не из-под наших PID-файлов.${NC}"
    echo -e "${YELLOW}   Проверь: ss -tlnp | grep -E '$(IFS="|"; echo "${PORTS[*]}")'${NC}"
    echo -e "${YELLOW}   Или: ./status.sh${NC}"
fi

# --- Старт (с пробросом аргументов) ---
echo -e "\n${YELLOW}▶ Запуск...${NC}"
"$PROJECT_ROOT/start_all.sh" "$@"
START_RC=$?

if [ $START_RC -ne 0 ]; then
    echo -e "${RED}❌ start_all.sh завершился с кодом $START_RC${NC}"
    exit $START_RC
fi

echo -e "\n${GREEN}✅ Перезапуск завершён${NC}"
