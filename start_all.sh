#!/bin/bash
# start_all.sh — Запуск микросервисов
#
# Использование:
#   ./start_all.sh [-d|-b|-f] [--bus-multi] [-h]
#
# Опции:
#   -d, --default    Инфраструктура (без бизнес-сервисов)
#   -b, --business   Инфраструктура + бизнес-сервисы (PJP, MG, BUS, PROTO)
#   -f, --full       Все сервисы + клиент
#   --bus-multi      N BUS (по группам из bus_instances_multi.yaml)
#   -h, --help       Справка
#
# BUS:
#   По умолчанию: 1 BUS (8005, все группы)
#   С --bus-multi: N BUS (8013-8019, по группам)

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"
mkdir -p "$LOG_DIR"

show_help() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}🚀 start_all.sh — Запуск микросервисов${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    echo -e "${YELLOW}Использование:${NC}"
    echo "  ./start_all.sh [-d|-b|-f] [--bus-multi] [-h]"
    echo ""
    echo -e "${YELLOW}Опции:${NC}"
    echo "  -d, --default    Инфраструктура (без бизнес-сервисов)"
    echo "  -b, --business   Инфраструктура + бизнес-сервисы (PJP, MG, BUS, PROTO)"
    echo "  -f, --full       Все сервисы + клиент"
    echo "  --bus-multi      N BUS (по группам из bus_instances_multi.yaml)"
    echo "  -h, --help       Справка"
    echo ""
    echo -e "${YELLOW}BUS:${NC}"
    echo "  По умолчанию: 1 BUS (8005, все группы)"
    echo "  С --bus-multi: N BUS (8013-8019, по группам)"
    echo ""
    echo -e "${BLUE}========================================${NC}"
}

MODE="default"
BUS_MULTI=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help) show_help; exit 0 ;;
        -d|--default) MODE="default"; shift ;;
        -b|--business) MODE="business"; shift ;;
        -f|--full) MODE="full"; shift ;;
        --bus-multi) BUS_MULTI=true; shift ;;
        *) echo -e "${RED}❌ Неизвестная опция: $1${NC}"; exit 1 ;;
    esac
done

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🚀 Запуск микросервисов (режим: ${YELLOW}$MODE${BLUE}, bus-multi: ${YELLOW}$BUS_MULTI${BLUE})${NC}"
echo -e "${BLUE}========================================${NC}"

check_port() {
    local port=$1
    lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1
}

start_service() {
    local service_name=$1
    local port=$2
    local service_path="$PROJECT_ROOT/$service_name"

    echo -e "\n${YELLOW}▶ Запуск $service_name на порту $port...${NC}"

    if [ ! -d "$service_path" ]; then
        echo -e "${YELLOW}⚠  Директория $service_name не найдена, пропускаем${NC}"
        return 0
    fi
    if [ ! -f "$service_path/run.sh" ]; then
        echo -e "${YELLOW}⚠  run.sh не найден в $service_name, пропускаем${NC}"
        return 0
    fi
    if check_port $port; then
        echo -e "${YELLOW}⚠  Порт $port занят. Пропускаем $service_name${NC}"
        return 0
    fi

    cd "$service_path"
    nohup ./run.sh > "$LOG_DIR/${service_name}.log" 2>&1 &
    local pid=$!
    echo $pid > "$LOG_DIR/${service_name}.pid"

    sleep 2
    if ps -p $pid > /dev/null 2>&1; then
        echo -e "${GREEN}✅ $service_name запущен (PID: $pid)${NC}"
    else
        echo -e "${RED}❌ $service_name не запустился${NC}"
    fi
}

start_bus_single() {
    local port=8005
    echo -e "\n${YELLOW}▶ Запуск BUS (single, порт $port)...${NC}"
    if check_port $port; then
        echo -e "${YELLOW}⚠  Порт $port занят${NC}"
        return 0
    fi
    cd "$PROJECT_ROOT/BUS_service"
    nohup ./run.sh --port $port --group default > "$LOG_DIR/BUS_service.log" 2>&1 &
    local pid=$!
    echo $pid > "$LOG_DIR/BUS_service.pid"
    sleep 2
    if ps -p $pid > /dev/null 2>&1; then
        echo -e "${GREEN}✅ BUS (single, :$port) запущен (PID: $pid)${NC}"
    else
        echo -e "${RED}❌ BUS не запустился${NC}"
    fi
}

start_bus_multi() {
    echo -e "\n${YELLOW}▶ Запуск N BUS (по группам из bus_instances_multi.yaml)...${NC}"

    local yaml="$PROJECT_ROOT/MG_service/config/bus_instances_multi.yaml"
    if [ ! -f "$yaml" ]; then
        echo -e "${RED}❌ $yaml не найден${NC}"
        return 0
    fi

    local lines
    lines=$(python3 -c "
import yaml
with open('$yaml') as f:
    data = yaml.safe_load(f)
for group_id, info in data.get('instances', {}).items():
    url = info.get('url', '')
    port = url.split(':')[-1].strip('/')
    if port.isdigit():
        print(f'{group_id} {port}')
" 2>/dev/null)

    if [ -z "$lines" ]; then
        echo -e "${RED}❌ Не удалось распарсить $yaml (PyYAML не установлен?)${NC}"
        return 0
    fi

    while read -r group port; do
        [ -z "$group" ] && continue
        if check_port $port; then
            echo -e "${YELLOW}⚠  Порт $port занят, пропускаем BUS ($group)${NC}"
            continue
        fi
        cd "$PROJECT_ROOT/BUS_service"
        nohup ./run.sh --port $port --group "$group" > "$LOG_DIR/BUS_${group}.log" 2>&1 &
        local pid=$!
        echo $pid > "$LOG_DIR/BUS_${group}.pid"
        echo -e "${GREEN}  ✅ BUS ($group, :$port) запущен (PID: $pid)${NC}"
    done <<< "$lines"

    sleep 2
}

start_ui_composer() {
    local service_name="UI_COMPOSER_SERVICE"
    local port=8020
    local service_path="$PROJECT_ROOT/$service_name"

    echo -e "\n${YELLOW}▶ Запуск UI Composer Service на порту $port...${NC}"

    if [ ! -d "$service_path" ]; then
        echo -e "${YELLOW}⚠  Директория $service_name не найдена${NC}"
        return 0
    fi
    if check_port $port; then
        echo -e "${YELLOW}⚠  Порт $port занят${NC}"
        return 0
    fi

    cd "$service_path"
    if [ ! -d "venv" ]; then
        python3 -m venv venv
        source venv/bin/activate
        pip install -r requirements.txt > /dev/null 2>&1
    fi
    nohup venv/bin/uvicorn src.main:app --host 0.0.0.0 --port $port > "$LOG_DIR/${service_name}.log" 2>&1 &
    local pid=$!
    echo $pid > "$LOG_DIR/${service_name}.pid"
    sleep 2
    if ps -p $pid > /dev/null 2>&1; then
        echo -e "${GREEN}✅ $service_name запущен (PID: $pid)${NC}"
    else
        echo -e "${RED}❌ $service_name не запустился${NC}"
    fi
}

start_gateway() {
    echo -e "\n${YELLOW}▶ Перезагрузка Nginx Gateway...${NC}"
    if sudo nginx -t 2>/dev/null; then
        if sudo systemctl reload nginx 2>/dev/null || sudo nginx -s reload 2>/dev/null; then
            echo -e "${GREEN}✅ Nginx Gateway перезагружен${NC}"
        else
            echo -e "${RED}❌ Не удалось перезагрузить nginx${NC}"
        fi
    else
        echo -e "${RED}❌ Ошибка в конфигурации nginx${NC}"
    fi
}

start_client() {
    echo -e "\n${YELLOW}▶ Запуск клиента...${NC}"
    local client_paths=(
        "$PROJECT_ROOT/CLIENT/build/production_client/ProductionClientApp"
        "$PROJECT_ROOT/CLIENT/build/ProductionClientApp"
        "$PROJECT_ROOT/CLIENT/build/IS_Client"
        "$PROJECT_ROOT/CLIENT/build/client"
    )
    local client_bin=""
    for path in "${client_paths[@]}"; do
        if [ -f "$path" ] && [ -x "$path" ]; then
            client_bin="$path"; break
        fi
    done
    if [ -z "$client_bin" ]; then
        echo -e "${YELLOW}⚠  Исполняемый файл клиента не найден${NC}"
        return 0
    fi
    cd "$(dirname "$client_bin")"
    nohup ./$(basename "$client_bin") > "$LOG_DIR/client.log" 2>&1 &
    echo $! > "$LOG_DIR/client.pid"
    echo -e "${GREEN}✅ Клиент запущен${NC}"
}

check_health() {
    local service=$1
    local port=$2
    if curl -s -o /dev/null -w "%{http_code}" "http://localhost:$port/health" 2>/dev/null | grep -q "200"; then
        echo -e "${GREEN}✅ $service OK${NC}"
    else
        echo -e "${RED}❌ $service FAILED${NC}"
    fi
}

abort_startup() {
    echo -e "\n${YELLOW}⚠  Прервано, останавливаем...${NC}"
    if [ -x "$PROJECT_ROOT/stop_all.sh" ]; then
        "$PROJECT_ROOT/stop_all.sh"
    fi
    exit 1
}

trap abort_startup INT TERM

# ---------- ИНФРАСТРУКТУРА ----------
echo -e "\n${BLUE}🏗  Инфраструктурные сервисы:${NC}"
start_service "AUTH_SERVICE" 8010
start_service "USER_service" 8000
start_service "PROJECT_service" 8001
start_service "NAVIGATION_SERVICE" 8009
start_service "DELEGATION_SERVICE" 8011
start_service "NOTIFICATION_SERVICE" 8012
start_ui_composer

# ---------- GATEWAY ----------
start_gateway

# ---------- БИЗНЕС-СЕРВИСЫ ----------
if [[ "$MODE" == "business" ]] || [[ "$MODE" == "full" ]]; then
    echo -e "\n${BLUE}📊 Бизнес-сервисы:${NC}"
    start_service "PJP_SERVICE" 8002

    # ВАЖНО: скопировать нужный конфиг bus_instances.yaml ДО запуска MG
    if [[ "$BUS_MULTI" == true ]]; then
        cp "$PROJECT_ROOT/MG_service/config/bus_instances_multi.yaml" \
           "$PROJECT_ROOT/MG_service/config/bus_instances.yaml"
        echo -e "${GREEN}✅ bus_instances.yaml ← bus_instances_multi.yaml${NC}"
    else
        # Восстановить single-конфиг (8005)
        if [ -f "$PROJECT_ROOT/MG_service/config/bus_instances_single.yaml" ]; then
            cp "$PROJECT_ROOT/MG_service/config/bus_instances_single.yaml" \
               "$PROJECT_ROOT/MG_service/config/bus_instances.yaml"
            echo -e "${GREEN}✅ bus_instances.yaml ← bus_instances_single.yaml${NC}"
        fi
    fi

    start_service "MG_service" 8003

    if [[ "$BUS_MULTI" == true ]]; then
        start_bus_multi
    else
        start_bus_single
    fi

    start_service "PROTO_service" 8004
fi

# ---------- КЛИЕНТ ----------
if [[ "$MODE" == "full" ]]; then
    start_client
fi

# ---------- HEALTH CHECK ----------
echo -e "\n${BLUE}🔍 Проверка здоровья...${NC}"
sleep 3
check_health "Auth" 8010
check_health "User" 8000
check_health "Project" 8001
check_health "Navigation" 8009
check_health "Delegation" 8011
check_health "Notification" 8012
check_health "UI Composer" 8020
check_health "Gateway" 8080

if [[ "$MODE" == "business" ]] || [[ "$MODE" == "full" ]]; then
    check_health "PJP" 8002
    check_health "MG" 8003

    if [[ "$BUS_MULTI" == true ]]; then
        for port in 8013 8014 8015 8016 8017 8018 8019; do
            check_health "BUS (:${port})" $port
        done
    else
        check_health "BUS" 8005
    fi

    check_health "PROTO" 8004
fi

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}✅ Запуск завершён${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "\n📊 Логи: $LOG_DIR/"
if [[ "$BUS_MULTI" == true ]]; then
    echo -e "${YELLOW}💡 BUS (multi): 8013-8019 (по группам)${NC}"
else
    echo -e "${YELLOW}💡 BUS (single): 8005 (все группы)${NC}"
fi
