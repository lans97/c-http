#ifndef REQUEST_INTERNAL_H
#define REQUEST_INTERNAL_H

#include "body.h"
#include "map/map.h"

struct http_request {
    sds                 method;
    sds                 uri;
    sds                 version;
    map*                header;
    struct  http_body   body;
};

void    free_sdsarr(sds* arr, int arrlen);

int     parse_request_line(struct http_request* req, const char* data, size_t len);
int     parse_headers(struct http_request* req, const char* data, size_t len);
int     parse_single_header(struct http_request* req, const char* line, size_t len);

#endif
