#!/bin/bash
# install.sh - Полная установка PJP Service на новом устройстве

set -e

echo "🚀 Установка PJP Service..."

# 1. Проверка Python
echo "📌 Проверка Python..."
if ! command -v python3 &> /dev/null; then
    echo "❌ Python3 не найден. Установите Python 3.12+"
    echo "   Ubuntu: sudo apt install python3 python3-venv python3-pip"
    echo "   Arch: sudo pacman -S python python-pip"
    exit 1
fi

# 2. Проверка PostgreSQL
echo "📌 Проверка PostgreSQL..."
if ! command -v psql &> /dev/null; then
    echo "⚠️  PostgreSQL не найден. Устанавливаем..."
    if command -v apt &> /dev/null; then
        sudo apt update
        sudo apt install -y postgresql postgresql-contrib
    elif command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm postgresql
    else
        echo "❌ Не удалось определить менеджер пакетов. Установите PostgreSQL вручную."
        exit 1
    fi
fi

# 3. Запуск PostgreSQL
echo "📌 Запуск PostgreSQL..."
if command -v systemctl &> /dev/null; then
    sudo systemctl start postgresql
    sudo systemctl enable postgresql
fi

# 4. Создание базы данных
echo "📌 Создание базы данных..."
sudo -u postgres psql -c "CREATE DATABASE pjp_service;" 2>/dev/null || echo "⚠️  База данных уже существует"

# 5. Создание .env файла
echo "📌 Создание .env файла..."
if [ ! -f .env ]; then
    cat > .env << 'ENVEOF'
DATABASE_URL=postgresql+asyncpg://postgres:password@localhost:5432/pjp_service
SECRET_KEY=your-super-secret-key-change-in-production
ALGORITHM=HS256
ACCESS_TOKEN_EXPIRE_MINUTES=30
ENVEOF
    echo "✅ .env создан"
else
    echo "✅ .env уже существует"
fi

# 6. Создание виртуального окружения
echo "📌 Создание виртуального окружения..."
if [ ! -d "venv" ]; then
    python3 -m venv venv
    echo "✅ Виртуальное окружение создано"
else
    echo "✅ Виртуальное окружение уже существует"
fi

# 7. Установка зависимостей
echo "📌 Установка зависимостей..."
source venv/bin/activate
pip install --upgrade pip > /dev/null 2>&1
pip install -r requirements.txt

# 8. Проверка подключения к БД
echo "📌 Проверка подключения к БД..."
if psql -U postgres -c "SELECT 1" > /dev/null 2>&1; then
    echo "✅ PostgreSQL работает"
else
    echo "⚠️  Проверьте настройки PostgreSQL"
    echo "   Пароль по умолчанию: password"
fi

# 9. Создание таблиц
echo "📌 Создание таблиц в БД..."
python -c "
import asyncio
from app.database import engine, Base
async def create():
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
asyncio.run(create())
print('✅ Таблицы созданы')
"

echo ""
echo "✅ УСТАНОВКА ЗАВЕРШЕНА!"
echo ""
echo "📋 ДАЛЬНЕЙШИЕ ДЕЙСТВИЯ:"
echo "   1. Проверьте настройки в .env (пароль PostgreSQL)"
echo "   2. Запустите сервис: ./run.sh"
echo "   3. Swagger UI: http://localhost:8002/docs"
echo ""
