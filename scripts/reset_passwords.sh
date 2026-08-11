#!/bin/bash
# reset_passwords.sh — массовый сброс паролей через API

set -e

BASE_URL="http://127.0.0.1:8000"
EMAIL="super.admin@company.com"
PASSWORD="newPassForSuperAdmin123!"
NEW_PASSWORD="LeaderPass456!"

echo "=== МАССОВЫЙ СБРОС ПАРОЛЕЙ ==="

# 1. Получаем токен суперадмина
echo "1. Логинимся как суперадмин..."
TOKEN=$(curl -s -X POST "${BASE_URL}/login" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"${EMAIL}\",\"password\":\"${PASSWORD}\"}" \
  | python3 -c "import sys, json; print(json.load(sys.stdin)['access_token'])")

if [ -z "$TOKEN" ] || [ "$TOKEN" == "null" ]; then
    echo "❌ Ошибка: не удалось получить токен!"
    exit 1
fi

echo "✅ Токен получен"

# 2. Список пользователей для сброса пароля
USERS=(
    "proto_pm.volkov"
    "proto_pm.novikov"
    "proto_pm.orlov"
    "supervisor.ivanov"
    "chief_engineer.smirnov"
    "industrialization.kuznetsov"
    "shop_assembly.kovalchuk"
    "shop_body.melnik"
    "kto_head.belousov"
    "economist_head.smirnova"
    "quality_head.nikolaev"
    "purchaser_head.petrov"
    "logistician_head.ivanova"
    "hr_head.sidorova"
    "design_pm_senior.vasin"
    "design_pm_junior.belov"
)

# 3. Для каждого пользователя сбрасываем пароль
echo ""
echo "2. Сброс паролей для руководителей..."

for USERNAME in "${USERS[@]}"; do
    echo -n "  → $USERNAME ... "
    
    # Получаем email пользователя
    EMAIL_USER=$(psql -U postgres -d user_service -t -c "SELECT email FROM users WHERE user_name = '$USERNAME';" | tr -d ' ')
    
    if [ -z "$EMAIL_USER" ] || [ "$EMAIL_USER" == "" ]; then
        echo "❌ пользователь не найден"
        continue
    fi
    
    # Сначала логинимся как пользователь (чтобы получить его токен)
    USER_TOKEN=$(curl -s -X POST "${BASE_URL}/login" \
        -H "Content-Type: application/json" \
        -d "{\"email\":\"${EMAIL_USER}\",\"password\":\"${NEW_PASSWORD}\"}" \
        | python3 -c "import sys, json; print(json.load(sys.stdin)['access_token'])" 2>/dev/null || echo "")
    
    if [ -z "$USER_TOKEN" ] || [ "$USER_TOKEN" == "null" ]; then
        # Если старый пароль не работает — используем суперадмина для смены
        # (потребуется админский эндпоинт, которого пока нет)
        echo "⚠️  старый пароль не подходит, пропускаем"
        continue
    fi
    
    # Меняем пароль (через API самого пользователя)
    RESPONSE=$(curl -s -X POST "${BASE_URL}/users/me/change-password" \
        -H "Authorization: Bearer $USER_TOKEN" \
        -H "Content-Type: application/json" \
        -d "{
            \"current_password\": \"${NEW_PASSWORD}\",
            \"new_password\": \"${NEW_PASSWORD}\",
            \"confirm_password\": \"${NEW_PASSWORD}\"
        }")
    
    if echo "$RESPONSE" | grep -q "message"; then
        echo "✅"
    else
        echo "❌ ошибка: $RESPONSE"
    fi
done

echo ""
echo "3. Проверка хешей в БД..."
psql -U postgres -d user_service -c "
SELECT user_name, length(password_hash) as hash_length 
FROM users 
WHERE user_name IN ('proto_pm.volkov', 'proto_pm.novikov', 'proto_pm.orlov')
ORDER BY user_name;
"

echo ""
echo "✅ Готово!"
