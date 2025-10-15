#pragma once

#include "http/body.h"
#include "http/results.h"
#include "http/version.h"
#include "map/map.h"

struct http_request {
    sds                 method;
    sds                 uri;
    http_version        version;
    map                *header;
    http_body           body;
};


ErrorMessage    parse_request_line(struct http_request* req, const char* data, size_t len);
ErrorMessage    parse_headers(struct http_request* req, const char* data, size_t len);
ErrorMessage    parse_single_header(struct http_request* req, const char* line, size_t len);

