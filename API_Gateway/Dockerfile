# Dockerfile
FROM nginx:alpine

# Удаляем дефолтный конфиг
RUN rm /etc/nginx/nginx.conf /etc/nginx/conf.d/default.conf 2>/dev/null || true

# Копируем наши конфиги
COPY nginx/nginx.conf /etc/nginx/nginx.conf
COPY nginx/conf.d/gateway.conf /etc/nginx/conf.d/gateway.conf

# Создаем директории для логов
RUN mkdir -p /var/log/nginx && chown nginx:nginx /var/log/nginx

# Проверяем конфиг
RUN nginx -t

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
