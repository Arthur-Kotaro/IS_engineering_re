#!/bin/bash

# Скрипт развёртывания для Linux

set -e

echo "=== Deploying Corporate Client for Linux ==="

# Директория для развёртывания
DEPLOY_DIR="deploy/linux"
mkdir -p $DEPLOY_DIR

# Копируем бинарные файлы
cp build/production_client/ProductionClient $DEPLOY_DIR/
cp build/engineering_client/EngineeringClient $DEPLOY_DIR/

# Копируем QML файлы
cp -r production_client/qml $DEPLOY_DIR/

# Копируем конфигурацию
cp -r production_client/config $DEPLOY_DIR/

# Копируем зависимости Qt (если нужен standalone)
if command -v linuxdeployqt &> /dev/null; then
    linuxdeployqt $DEPLOY_DIR/ProductionClient -bundle-non-qt-libs
fi

echo "=== Deployment completed to $DEPLOY_DIR ==="
