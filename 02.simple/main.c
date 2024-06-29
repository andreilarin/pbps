#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include "auth.h"

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
// В функцию-обработчик клиентского запроса должен передаваться идентифкатор SSL
void respond(int, SSL *);
// Формирование SSL-контекста сервера
static SSL_CTX *get_server_context(const char *, const char *, const char *);

int main(int argc, char* argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;    
	
	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT,"10000");

	int slot=0;
	int rc;


	//Parsing the command line arguments
	while ((c = getopt (argc, argv, "p:r:")) != -1)
		switch (c)
		{
			case 'r':
				ROOT = malloc(strlen(optarg));
				strcpy(ROOT,optarg);
				break;
			case 'p':
				strcpy(PORT,optarg);
				break;
			case '?':
				fprintf(stderr,"Wrong arguments given!\n");
				exit(1);
			default:
				exit(1);
		}
	
	printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
	printf("============================================\n");
	printf("\n");


	// Инициализация OpenSSL
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
	// Инициализация контекста сервера
	SSL_CTX *ctx; // контекст сервера
	SSL *ssl;     // SSL-обработчик подключения
	if (!(ctx = get_server_context("./keys/ca/ca_cert.pem", "./keys/server/server_cert.pem", "./keys/server/private/server_key.pem"))) {
		exit(0);
	}

	// Setting all elements to -1: signifies there is no client connected
	int i;
	for (i=0; i<CONNMAX; i++)
		clients[i]=-1;
	startServer(PORT);

	// ACCEPT connections
	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);
		/* Получение SSL-обработчика из контекста сервера */
		if (!(ssl = SSL_new(ctx))) {
				fprintf(stderr, "%s", "Не могу получить ссылку на SSL-обработчик из SSL-контекста сервера\n");
				close(clients[slot]);
				continue;
		}

		/* Сопоставление установленного сетевого соединения с SSL-обработчиком*/
		SSL_set_fd(ssl, clients[slot]);

	 	/* Выполнение согласования параметров SSL-соединения */
		if ((rc = SSL_accept(ssl)) != 1) {
				fprintf(stderr, "Согласование SSL-параметров не выполнено\n");
				if (rc != 0) {
					SSL_shutdown(ssl);
				}
				SSL_free(ssl);
				continue;
		} else {
				fprintf(stderr, "Согласование SSL-параметров выполнено успешно от: %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
				if (clients[slot] < 0) {
					perror("accept() error");
					exit(1);
				} else {
					if (fork() == 0) {
							close(listenfd);
							respond(slot, ssl);
							close(clients[slot]);
							clients[slot] = -1;
							exit(0);
					} else {
							close(clients[slot]);
					}
				}
		}

		while (clients[slot] != -1 ) 
			slot = (slot + 1) % CONNMAX;
		SSL_shutdown(ssl);
    		SSL_free(ssl);

	}

	return 0;
}

// Формирования SSL-контекста для сервера
static SSL_CTX *get_server_context(const char *ca_pem, const char *cert_pem, const char *key_pem) {
	SSL_CTX *ctx;
	/* Формирование контекста с параметрами по-умолчанию */
	if (!(ctx = SSL_CTX_new(TLS_server_method()))) {
		fprintf(stderr, "Ошибка SSL_CTX_new\n");
		return NULL;
	}
	/* Установка пути к сертифкату CA */
	if (SSL_CTX_load_verify_locations(ctx, ca_pem, NULL) != 1) {
		fprintf(stderr, "Не могу определить путь к файлу сертификата CA\n");
		SSL_CTX_free(ctx);
		return NULL;
	}
	/* Установка серверного сертификата, подписанного CA */
	if (SSL_CTX_use_certificate_file(ctx, cert_pem, SSL_FILETYPE_PEM) != 1) {
		fprintf(stderr, "Не могу назначить сертификат сервера\n");
		SSL_CTX_free(ctx);
		return NULL;
	}
	/* Установка приватного ключа сервера */
	if (SSL_CTX_use_PrivateKey_file(ctx, key_pem, SSL_FILETYPE_PEM) != 1) {
		fprintf(stderr, "Не могу назначить приватный ключ сервера\n");
		SSL_CTX_free(ctx);
		return NULL;
	}
	/* Проверка соответсвия приватного ключа и сертификата сервера */
	if (SSL_CTX_check_private_key(ctx) != 1) {
		fprintf(stderr, "Сертифкат сервера и его приватный ключ не соответсвуют друг другу\n");
		SSL_CTX_free(ctx);
		return NULL;
	}
	/* Режим выполнения операций чтения-записи только после успешного (пере)согласования параметров */
	SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
	/* Принимаем только сертифкаты подписанные самим удостоверяющим центром */
	SSL_CTX_set_verify_depth(ctx, 1);
	
	// Запрашиваем клиентские сертификаты
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

	/* Возвращаем контекст */
	return ctx;
}

//start server
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
}

//client connection
void respond(int n, SSL* ssl)
{
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;

	// Получение информации о клиентском сертификате
	X509 *client_cert = SSL_get_peer_certificate(ssl);
	if (client_cert) {
		// Получение информации о сертификате
		char *subject = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
		printf("Объект сертификата пользователя: %s\n", subject);

		// Получаем CN из сертификата для аутентификации пользователя
		char *cn = NULL;
		X509_NAME *name = X509_get_subject_name(client_cert);
		int lastpos = -1;
		while ((lastpos = X509_NAME_get_index_by_NID(name, NID_commonName, lastpos)) != -1) {
			X509_NAME_ENTRY *e = X509_NAME_get_entry(name, lastpos);
			ASN1_STRING *d = X509_NAME_ENTRY_get_data(e);
			cn = (char *) ASN1_STRING_data(d);
		}

		if (cn) {
			printf("CN клиента: %s\n", cn);
			if (pam_authenticate_user(cn) != 0) {
				fprintf(stderr, "Authentication failed for user: %s\n", cn);
				SSL_write(ssl, "HTTP/1.0 403 Forbidden\n", 23);
				X509_free(client_cert);
				return;
			}
		} else {
			fprintf(stderr, "No CN found in client certificate\n");
			SSL_write(ssl, "HTTP/1.0 403 Forbidden\n", 23);
			X509_free(client_cert);
			return;
		}

		OPENSSL_free(subject);
		X509_free(client_cert);
	} else {
		fprintf(stderr, "Client did not present a certificate.\n");
		SSL_write(ssl, "HTTP/1.0 403 Forbidden\n", 23);
		return;
	}

	memset( (void*)mesg, (int)'\0', 99999 );

	// rcvd=recv(clients[n], mesg, 99999, 0);
	rcvd = SSL_read(ssl, mesg, 99999);

	if (rcvd<0)    // receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"Client disconnected unexpectedly.\n");
	else    // message received
	{
		printf("%s", mesg);
		reqline[0] = strtok (mesg, " \t\n");
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
			if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
			{
				//write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
				SSL_write(ssl, "HTTP/1.0 400 Bad Request\n", 25);
			}
			else
			{
				if ( strncmp(reqline[1], "/\0", 2)==0 )
					reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], reqline[1]);
				printf("file: %s\n", path);
				printf("============================================\n");
				printf("\n");

				if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
				{
					SSL_write(ssl, "HTTP/1.0 200 OK\n\n", 17);
					while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
						SSL_write (ssl, data_to_send, bytes_read);
				}
				else    
					SSL_write(ssl, "HTTP/1.0 404 Not Found\n", 23); 
			}
		}
	}

	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
}
