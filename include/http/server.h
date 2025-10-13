#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

typedef struct http_server http_server;

http_server* http_server_new();
void start_and_listen();

#endif

