#pragma once

#include "sds.h"
#include <netinet/in.h>

typedef struct http_connection {
    int     fd;
    sds     buffer;
    size_t  content_length;
    bool    header_parsed;
    bool    keep_alive;
} http_connection;

http_connection*    http_connection_new();
int                 http_connection_bindAndListen(http_connection *this,
                                                  int port);
void                http_connection_delete(http_connection* this);

