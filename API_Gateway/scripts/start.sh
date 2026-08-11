#!/bin/bash
# scripts/start.sh
echo "Starting API Gateway..."
docker-compose up -d --build
echo "Gateway running on http://localhost:8080"
echo ""
echo "Available routes:"
echo "  http://localhost:8080/api/v1/auth/login  -> User Service"
echo "  http://localhost:8080/api/v1/users/me    -> User Service"
echo "  http://localhost:8080/api/v1/projects/   -> Project Service"
echo "  http://localhost:8080/health             -> Gateway health"
