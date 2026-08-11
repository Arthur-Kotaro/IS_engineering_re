#!/bin/bash

# Скрипт запуска тестов

set -e

echo "=== Running Tests ==="

cd build

# Запуск всех тестов
ctest --output-on-failure --verbose

# Запуск отдельных тестов
echo ""
echo "=== Library Tests ==="
./tests/LibraryTests --gtest_color=yes

echo ""
echo "=== Engineering Client Tests ==="
./tests/EngineeringClientTests --gtest_color=yes

echo "=== All tests completed ==="
