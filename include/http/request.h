#ifndef REQUEST_H
#define REQUEST_H

#include <stddef.h>
struct http_request;
typedef struct http_request http_request;

http_request*   http_request_fromBytes(const char* data, size_t len);
void            http_request_delete(http_request* this);

#endif
