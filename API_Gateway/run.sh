#!/bin/bash
# run.sh — Перезапуск API Gateway (NGINX)

echo "🔁 Restarting API Gateway..."
sudo nginx -t && sudo systemctl reload nginx
echo "✅ API Gateway restarted on port 8080"
