#ifndef HTTP_SERVER_INTERNAL_H
#define HTTP_SERVER_INTERNAL_H

#include "sds.h"
#include <netinet/in.h>

typedef struct http_connection {
    int     fd;
    sds     buffer;
    size_t  content_length;
    bool    header_parsed;
    bool    keep_alive;
} http_connection;

#endif
