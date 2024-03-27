#include "httpd.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdlib.h>
#include "logger.h"

#define CHUNK_SIZE 1024 // read 1024 bytes at a time

// Public directory settings
#define PUBLIC_DIR_DEFAULT "./webroot"
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"

char* PUBLIC_DIR = PUBLIC_DIR_DEFAULT;

int main(int c, char **v) {
  char *port = c == 1 ? "8000" : v[2];
  PUBLIC_DIR = v[4];
  serve_forever(port);
  return 0;
}

int file_exists(const char *file_name) {
  int exists;
  exists = (open(file_name, O_RDONLY) != -1);

  return exists;
}

int read_file(const char *file_name) {
  char buf[CHUNK_SIZE];
  FILE *file;
  size_t nread;
  int err = 1;

  file = fopen(file_name, "r");

  if (file) {
    while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
      fwrite(buf, 1, nread, stdout);

    err = ferror(file);
    fclose(file);
  }
  return err;
}

void route() {
  ROUTE_START()

  GET("/") {
    char *index_html;
    index_html = malloc(strlen(PUBLIC_DIR) + strlen(INDEX_HTML));
    sprintf(index_html, "%s%s", PUBLIC_DIR, INDEX_HTML);

    HTTP_200;
    if (file_exists(index_html)) {
      read_file(index_html);
    } else {
      printf("Hello! You are using %s\n\n", request_header("User-Agent"));
    }
    free(index_html);
    log_request(200);
  }

  GET("/test") {
    HTTP_200;
    printf("List of request headers:\n\n");

    header_t *h = request_headers();

    while (h->name) {
      printf("%s: %s\n", h->name, h->value);
      h++;
    }
    log_request(200);
  }

  POST("/") {
    HTTP_201;
    printf("Wow, seems that you POSTed %d bytes.\n", payload_size);
    printf("Fetch the data using `payload` variable.\n");
    if (payload_size > 0)
      printf("Request body: %s", payload);
    log_request(201);
  }

  GET(uri) {
    char *file_name;
    file_name = malloc(strlen(PUBLIC_DIR) + strlen(uri));
    sprintf(file_name, "%s%s", PUBLIC_DIR, uri);

    if (file_exists(file_name)) {
      HTTP_200;
      read_file(file_name);
      log_request(200);
    } else {
      HTTP_404;
      sprintf(file_name, "%s%s", PUBLIC_DIR, NOT_FOUND_HTML);
      if (file_exists(file_name))
        read_file(file_name);
      log_request(404);
    }
    free(file_name);
  }

  ROUTE_END()
}
