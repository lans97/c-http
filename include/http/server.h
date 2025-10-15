#pragma once

typedef struct http_server http_server;

http_server* http_server_new();
void start_and_listen();
