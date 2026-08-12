#!/bin/bash
# restart_all.sh — Перезапуск всех сервисов

./stop_all.sh
sleep 2
./start_all.sh
