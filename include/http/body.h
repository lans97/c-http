#ifndef HTTP_BODY_H
#define HTTP_BODY_H

#include <stddef.h>

typedef struct http_body {
    void*   data;
    size_t  length;
} http_body;

#endif
