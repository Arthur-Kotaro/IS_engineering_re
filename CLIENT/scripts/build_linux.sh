#!/bin/bash

# Скрипт сборки проекта для Linux

set -e

echo "=== Building Corporate Client ==="

# Создаем директорию для сборки
mkdir -p build && cd build

# Настройка CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_PRODUCTION_CLIENT=ON \
    -DBUILD_ENGINEERING_CLIENT=ON \
    -DBUILD_TESTS=ON

# Сборка
make -j$(nproc)

echo "=== Build completed ==="
echo "Production client: build/production_client/ProductionClient"
echo "Engineering client: build/engineering_client/EngineeringClient"
