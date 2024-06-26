# Тема 12.
Реализация аутентификации пользователей на HTTP-сервере с использованием SSL-сертификатов. Пользователи хранятся в системной базе УЗ (ключевые слова: HTTP Auth, Crypto, PKI, PAM, libpam)

## Сборка/запуск

- Сборка

~~~
sudo bash script.sh install
~~~

- Тестирование

~~~
sudo curl -k --cert /var/www/simple/keys/client/client_cert.pem --key /var/www/simple/keys/client/client_key.pem https://localhost:8080
~~~

