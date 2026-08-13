#!/bin/bash
# clean.sh - Очистка проекта

echo "🧹 Очистка PJP Service..."

read -p "Удалить виртуальное окружение? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf venv
    echo "✅ Виртуальное окружение удалено"
fi

read -p "Удалить кэш Python? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null
    find . -type f -name "*.pyc" -delete 2>/dev/null
    echo "✅ Кэш удален"
fi

read -p "Удалить базу данных? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    sudo -u postgres psql -c "DROP DATABASE IF EXISTS pjp_service;" 2>/dev/null
    echo "✅ База данных удалена"
fi

echo "🧹 Очистка завершена!"
