echo "Генерация сертификатов и ключей CA"

USER_NAME=$1
USER_PASSWORD=$2

SUBJ_CA="/C=RU/ST=./L=./O=./OU=./CN=ca_cert"

mkdir -p keys/ca keys/server keys/server/private keys/client

openssl genrsa -out keys/ca/ca_key.pem 2048

openssl req -new -x509 -key keys/ca/ca_key.pem -out keys/ca/ca_cert.pem -days 365 -subj $SUBJ_CA

echo "Генерация сертификатов и ключей сервера"

SUBJ_SERVER="/C=RU/ST=./L=./O=./OU=./CN=server_cert"

openssl genrsa -out keys/server/private/server_key.pem 2048

openssl req -new -key keys/server/private/server_key.pem -out keys/server/private/server_csr.pem -subj $SUBJ_SERVER

openssl x509 -req -in keys/server/private/server_csr.pem -CA keys/ca/ca_cert.pem -CAkey keys/ca/ca_key.pem -CAcreateserial -out keys/server/server_cert.pem -days 365

openssl verify -CAfile keys/ca/ca_cert.pem keys/server/server_cert.pem

echo
echo "Сертификаты и ключи сервера созданы, генерация сертификата и ключа клиента"
echo

SUBJ_CLIENT="/C=RU/ST=./L=./O=./OU=./CN=$USER_NAME/userPassword=$USER_PASSWORD"

openssl genrsa -out keys/client/client_key.pem 2048

openssl req -new -key keys/client/client_key.pem -out keys/client/client_csr.pem -subj $SUBJ_CLIENT

openssl x509 -req -in keys/client/client_csr.pem -CA keys/ca/ca_cert.pem -CAkey keys/ca/ca_key.pem -CAcreateserial -out keys/client/client_cert.pem -days 365

openssl verify -CAfile keys/ca/ca_cert.pem keys/client/client_cert.pem

echo
echo "Сертификат и ключ клиента созданы"
