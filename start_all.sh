#!/bin/bash
# start_all.sh — Запуск всех микросервисов из корня проекта

# Убираем set -e чтобы скрипт не прерывался при ошибках
# set -e

# Цвета
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$PROJECT_ROOT/logs"
mkdir -p "$LOG_DIR"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}🚀 Запуск всех микросервисов${NC}"
echo -e "${BLUE}========================================${NC}"

# Функция проверки порта
check_port() {
    local port=$1
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1 ; then
        return 0
    else
        return 1
    fi
}

# Функция запуска сервиса
start_service() {
    local service_name=$1
    local port=$2
    local service_path="$PROJECT_ROOT/$service_name"
    
    echo -e "\n${YELLOW}▶ Запуск $service_name на порту $port...${NC}"
    
    if [ ! -d "$service_path" ]; then
        echo -e "${YELLOW}⚠️  Директория $service_name не найдена, пропускаем${NC}"
        return 0
    fi
    
    if [ ! -f "$service_path/run.sh" ]; then
        echo -e "${YELLOW}⚠️  run.sh не найден в $service_name, пропускаем${NC}"
        return 0
    fi
    
    if check_port $port; then
        echo -e "${YELLOW}⚠️  Порт $port занят. Пропускаем $service_name${NC}"
        return 0
    fi
    
    cd "$service_path"
    nohup ./run.sh > "$LOG_DIR/${service_name}.log" 2>&1 &
    local pid=$!
    echo $pid > "$LOG_DIR/${service_name}.pid"
    
    sleep 2
    
    if ps -p $pid > /dev/null 2>&1; then
        echo -e "${GREEN}✅ $service_name запущен (PID: $pid)${NC}"
        echo -e "   Лог: $LOG_DIR/${service_name}.log"
    else
        echo -e "${RED}❌ $service_name не запустился. Проверьте лог${NC}"
        return 0
    fi
}

# Функция запуска API Gateway
start_gateway() {
    echo -e "\n${YELLOW}▶ Запуск API Gateway (nginx)...${NC}"
    
    if sudo nginx -t 2>/dev/null; then
        if sudo systemctl reload nginx 2>/dev/null || sudo nginx -s reload 2>/dev/null; then
            echo -e "${GREEN}✅ API Gateway перезапущен (порт 8080)${NC}"
        else
            echo -e "${RED}❌ Не удалось перезапустить nginx${NC}"
            return 0
        fi
    else
        echo -e "${RED}❌ Ошибка в конфигурации nginx${NC}"
        return 0
    fi
}

# Функция запуска клиента
start_client() {
    echo -e "\n${YELLOW}▶ Запуск клиента...${NC}"
    
    # Ищем клиент в разных возможных местах
    local client_paths=(
        "$PROJECT_ROOT/CLIENT/build/production_client/ProductionClientApp"
        "$PROJECT_ROOT/CLIENT/build/ProductionClientApp"
        "$PROJECT_ROOT/CLIENT/build/IS_Client"
        "$PROJECT_ROOT/CLIENT/build/client"
    )
    
    local client_bin=""
    for path in "${client_paths[@]}"; do
        if [ -f "$path" ] && [ -x "$path" ]; then
            client_bin="$path"
            break
        fi
    done
    
    if [ -z "$client_bin" ]; then
        echo -e "${YELLOW}⚠️  Исполняемый файл клиента не найден, пропускаем${NC}"
        echo -e "   Искали в: ${client_paths[*]}"
        return 0
    fi
    
    cd "$(dirname "$client_bin")"
    nohup ./$(basename "$client_bin") > "$LOG_DIR/client.log" 2>&1 &
    local pid=$!
    echo $pid > "$LOG_DIR/client.pid"
    echo -e "${GREEN}✅ Клиент запущен (PID: $pid)${NC}"
    echo -e "   Лог: $LOG_DIR/client.log"
    echo -e "   Бинарь: $client_bin"
}

# Остановка всех сервисов
stop_all() {
    echo -e "\n${YELLOW}🛑 Остановка всех сервисов...${NC}"
    
    for pid_file in "$LOG_DIR"/*.pid; do
        if [ -f "$pid_file" ]; then
            local pid=$(cat "$pid_file")
            if ps -p $pid > /dev/null 2>&1; then
                kill -TERM $pid 2>/dev/null || kill -KILL $pid 2>/dev/null
                echo "   Остановлен процесс $pid"
            fi
            rm -f "$pid_file"
        fi
    done
    
    pkill -f "uvicorn.*--port" 2>/dev/null || true
    echo -e "${GREEN}✅ Все сервисы остановлены${NC}"
}

# Обработка сигналов
trap stop_all EXIT INT TERM

# Запуск сервисов
echo -e "\n${BLUE}📋 Запуск сервисов:${NC}"
echo "   • User Service (8000)"
echo "   • Project Service (8001)"
echo "   • PJP Service (8002)"
echo "   • Mastergraphics Service (8003)"
echo "   • PROTO Service (8004)"
echo "   • Navigation Service (8009)"
echo "   • API Gateway (8080)"
echo "   • Клиент"

start_service "USER_service" 8000
start_service "PROJECT_service" 8001
start_service "PJP_SERVICE" 8002
start_service "MG_service" 8003
start_service "PROTO_service" 8004
start_service "NAVIGATION_SERVICE" 8009

start_gateway
start_client

# Health Check
echo -e "\n${BLUE}🔍 Проверка здоровья...${NC}"
sleep 3

check_health() {
    local service=$1
    local port=$2
    if curl -s -o /dev/null -w "%{http_code}" "http://localhost:$port/health" 2>/dev/null | grep -q "200"; then
        echo -e "${GREEN}✅ $service OK${NC}"
    else
        echo -e "${RED}❌ $service FAILED${NC}"
    fi
}

check_health "User" 8000
check_health "Project" 8001
check_health "PJP" 8002
check_health "MG" 8003
check_health "PROTO" 8004
check_health "Navigation" 8009
check_health "Gateway" 8080

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}✅ Все сервисы запущены!${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "\n📊 Логи: $LOG_DIR/"
echo -e "🛑 Для остановки нажмите Ctrl+C"
echo -e "\n🌐 API Gateway: http://localhost:8080"

# Ждем завершения
wait
