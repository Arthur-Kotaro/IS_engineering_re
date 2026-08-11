#!/bin/bash

# Скрипт для исправления структуры проекта
# Удаляет дубликаты и исправляет опечатки

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo "========================================="
echo "  ИСПРАВЛЕНИЕ СТРУКТУРЫ ПРОЕКТА"
echo "========================================="
echo ""

# 1. Удаление дублирующейся директории
if [ -d "common/include/user_service_client" ]; then
    print_warning "Найдена дублирующаяся директория: common/include/user_service_client"
    
    # Переносим файлы если они есть
    if [ -d "common/include/userserviceclient" ]; then
        print_info "Целевая директория существует, переносим файлы..."
        cp -rn common/include/user_service_client/* common/include/userserviceclient/ 2>/dev/null || true
    else
        print_info "Переименовываем директорию..."
        mv common/include/user_service_client common/include/userserviceclient
    fi
    
    # Удаляем старую директорию
    rm -rf common/include/user_service_client
    print_success "Удалена дублирующаяся директория"
fi

# 2. Исправление опечаток в QML файлах
fix_qml_filename() {
    local old_name="$1"
    local new_name="$2"
    local dir="$3"
    
    if [ -f "$dir/$old_name" ]; then
        mv "$dir/$old_name" "$dir/$new_name"
        print_success "Переименован: $old_name -> $new_name"
    fi
}

# Исправляем опечатки в components
fix_qml_filename "MastergaphStatusCard.qml" "MastergraphStatusCard.qml" "production_client/qml/components"
fix_qml_filename "PasswordExpirityWarning.qml" "PasswordExpiryWarning.qml" "production_client/qml/components"
fix_qml_filename "MastergaphDelegate.qml" "MastergraphDelegate.qml" "production_client/qml/delegates"
fix_qml_filename "ProjectView.qml" "ProjectsView.qml" "production_client/qml/views"

# 3. Проверка на наличие .cpp/.h файлов без пар
check_missing_pairs() {
    local dir="$1"
    local ext1="$2"
    local ext2="$3"
    
    for file in "$dir"/*."$ext1"; do
        [ -f "$file" ] || continue
        base="${file%.$ext1}"
        if [ ! -f "$base.$ext2" ]; then
            print_warning "Найден $ext1 файл без пары $ext2: $base.$ext1"
        fi
    done
}

print_info "Проверка пар файлов в production_client/src/core..."
check_missing_pairs "production_client/src/core" "cpp" "h"
check_missing_pairs "production_client/src/core" "h" "cpp"

print_info "Проверка пар файлов в production_client/src/qml_bridge..."
check_missing_pairs "production_client/src/qml_bridge" "cpp" "h"
check_missing_pairs "production_client/src/qml_bridge" "h" "cpp"

# 4. Проверка наличия всех необходимых директорий
ensure_directory() {
    if [ ! -d "$1" ]; then
        mkdir -p "$1"
        print_success "Создана недостающая директория: $1"
    fi
}

print_info "Проверка наличия всех необходимых директорий..."
ensure_directory "production_client/qml/delegates"
ensure_directory "production_client/qml/utils"
ensure_directory "production_client/assets/images/icons"
ensure_directory "production_client/assets/images/backgrounds"
ensure_directory "tests/qml"
ensure_directory "scripts"

# 5. Проверка наличия скриптов
ensure_script() {
    if [ ! -f "scripts/$1" ]; then
        print_warning "Отсутствует скрипт: scripts/$1"
    fi
}

print_info "Проверка наличия скриптов..."
ensure_script "build_linux.sh"
ensure_script "run_tests.sh"
ensure_script "deploy_linux.sh"
ensure_script "mock_server.py"

# 6. Создание недостающих файлов конфигурации
if [ ! -f "production_client/config/logging_config.ini" ]; then
    cat > "production_client/config/logging_config.ini" << 'EOF'
[loggers]
keys=root

[handlers]
keys=consoleHandler,fileHandler

[formatters]
keys=simpleFormatter

[logger_root]
level=INFO
handlers=consoleHandler,fileHandler

[handler_consoleHandler]
class=StreamHandler
level=INFO
formatter=simpleFormatter
args=(sys.stdout,)

[handler_fileHandler]
class=FileHandler
level=DEBUG
formatter=simpleFormatter
args=('client.log', 'a')

[formatter_simpleFormatter]
format=%(asctime)s - %(name)s - %(levelname)s - %(message)s
EOF
    print_success "Создан logging_config.ini"
fi

# 7. Очистка от временных файлов
print_info "Очистка от временных файлов..."
find . -name "*.swp" -delete 2>/dev/null || true
find . -name "*.swo" -delete 2>/dev/null || true
find . -name "*~" -delete 2>/dev/null || true
find . -name "*.log" -delete 2>/dev/null || true
find . -name "*.tmp" -delete 2>/dev/null || true
print_success "Временные файлы удалены"

# 8. Статистика
echo ""
echo "========================================="
echo "  ИТОГОВАЯ СТАТИСТИКА"
echo "========================================="

total_dirs=$(find . -type d -not -path '*/\.*' | wc -l)
total_files=$(find . -type f -not -path '*/\.*' | wc -l)

echo "Всего директорий: $total_dirs"
echo "Всего файлов: $total_files"

echo ""
print_success "Структура проекта исправлена!"
echo ""

# 9. Показать исправленную структуру
print_info "Текущая структура common/include:"
ls -la common/include/ 2>/dev/null || echo "  (пусто)"

echo ""
print_info "Рекомендации:"
echo "  1. Проверьте содержимое common/include/userserviceclient/"
echo "  2. Убедитесь, что все .h файлы имеют соответствующие .cpp"
echo "  3. Запустите сборку для проверки: ./scripts/build_linux.sh"
