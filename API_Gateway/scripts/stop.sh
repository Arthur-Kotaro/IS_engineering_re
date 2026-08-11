#!/bin/bash
# scripts/stop.sh
echo "Stopping API Gateway..."
docker-compose down
echo "Gateway stopped"
