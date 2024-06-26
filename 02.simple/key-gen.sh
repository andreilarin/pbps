echo "-----------------Генерация сертификатов и ключей сервера-----------------"

SUBJ="/C=RU/ST=./L=./O=./OU=./CN=Name"

mkdir -p keys/ca keys/server keys/server/private keys/client

openssl genrsa -out keys/ca/ca_key.pem 2048

openssl req -new -x509 -key keys/ca/ca_key.pem -out keys/ca/ca_cert.pem -days 365 -subj $SUBJ

openssl genrsa -out keys/server/private/server_key.pem 2048

openssl req -new -key keys/server/private/server_key.pem -out keys/server/private/server_csr.pem -subj $SUBJ

openssl x509 -req -in keys/server/private/server_csr.pem -CA keys/ca/ca_cert.pem -CAkey keys/ca/ca_key.pem -CAcreateserial -out keys/server/server_cert.pem -days 365

openssl verify -CAfile keys/ca/ca_cert.pem keys/server/server_cert.pem

echo
echo "-----------------Сертификаты и ключи сервера созданы, генерация сертификата и ключа клиента-----------------"
echo

openssl genpkey -algorithm RSA -out keys/client/client_key.pem -aes256

openssl req -new -key keys/client/client_key.pem -out keys/client/client_csr.pem -subj $SUBJ

openssl x509 -req -in keys/client/client_csr.pem -CA keys/ca/ca_cert.pem -CAkey keys/ca/ca_key.pem -CAcreateserial -out keys/client/client_cert.pem -days 365 -sha256

echo
echo "-----------------Сертификат и ключ клиента созданы-----------------"
