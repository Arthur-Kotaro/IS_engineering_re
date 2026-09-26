#!/bin/bash
# deploy.sh — копирует конфиги nginx из репозитория в /etc/nginx/ с бэкапом.

set -e

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
BACKUP_DIR="/etc/nginx/backup-$(date +%Y%m%d-%H%M%S)"

echo "=== Nginx deploy ==="

if [ "$EUID" -ne 0 ]; then
    echo "Запустите через sudo"
    exit 1
fi

echo "Бэкап текущих конфигов в $BACKUP_DIR"
mkdir -p "$BACKUP_DIR"
cp -a /etc/nginx/nginx.conf "$BACKUP_DIR/nginx.conf" 2>/dev/null || true
cp -a /etc/nginx/sites-available/gateway.conf "$BACKUP_DIR/gateway.conf" 2>/dev/null || true

echo "Копирование nginx.conf..."
cp "$REPO_DIR/nginx.conf" /etc/nginx/nginx.conf

echo "Копирование gateway.conf..."
cp "$REPO_DIR/gateway.conf" /etc/nginx/sites-available/gateway.conf

echo "Проверка конфигурации..."
nginx -t

echo "Перезагрузка nginx..."
systemctl reload nginx

echo "OK"
