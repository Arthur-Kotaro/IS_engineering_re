#!/usr/bin/env python3
# fix_passwords.py — массовое обновление паролей через bcrypt

import bcrypt
import psycopg2
from psycopg2.extras import RealDictCursor

# Подключение к БД
conn = psycopg2.connect(
    host="localhost",
    database="user_service",
    user="postgres",
    password="password"
)
cur = conn.cursor(cursor_factory=RealDictCursor)

# Пароль для всех руководителей
NEW_PASSWORD = "LeaderPass456!"

# Генерируем правильный хеш
hashed = bcrypt.hashpw(NEW_PASSWORD.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')
print(f"✅ Хеш сгенерирован, длина: {len(hashed)}")

# Список пользователей для обновления
users = [
    'proto_pm.volkov',
    'proto_pm.novikov',
    'proto_pm.orlov',
    'supervisor.ivanov',
    'chief_engineer.smirnov',
    'industrialization.kuznetsov',
    'shop_assembly.kovalchuk',
    'shop_body.melnik',
    'kto_head.belousov',
    'economist_head.smirnova',
    'quality_head.nikolaev',
    'purchaser_head.petrov',
    'logistician_head.ivanova',
    'hr_head.sidorova',
    'design_pm_senior.vasin',
    'design_pm_junior.belov'
]

print("\nОбновление паролей...")
for username in users:
    cur.execute(
        "UPDATE users SET password_hash = %s WHERE user_name = %s",
        (hashed, username)
    )
    if cur.rowcount > 0:
        print(f"  ✅ {username} обновлён")
    else:
        print(f"  ⚠️ {username} не найден")

conn.commit()

# Проверка
print("\nПроверка длины хешей:")
cur.execute("""
    SELECT user_name, length(password_hash) as hash_length
    FROM users
    WHERE user_name LIKE 'proto_pm.%'
    ORDER BY user_name;
""")
for row in cur.fetchall():
    print(f"  {row['user_name']}: {row['hash_length']} символов")

cur.close()
conn.close()
print("\n✅ Готово! Все хеши теперь длиной 60 символов.")
