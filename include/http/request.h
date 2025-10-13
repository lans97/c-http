#ifndef REQUEST_H
#define REQUEST_H

#include "body.h"
#include "http/utils.h"
#include <stddef.h>
struct http_request;
typedef struct http_request http_request;

http_request *http_request_fromBytes(const char *data, size_t len);
void http_request_delete(http_request *this);

const char *http_request_Method(http_request *this);
const char *http_request_Uri(http_request *this);
http_version *http_request_Version(http_request *this);

void http_request_HeaderSetValue(http_request *this, const char *headerKey,
                                 const char *headerValue);
const char *http_request_HeaderGetValue(http_request *this,
                                        const char *headerKey);

const char **http_request_HeaderKeys(http_request *this, size_t *keys_length);
bool http_request_HeaderContains(http_request *this, const char *headerKey);

http_body *http_request_Body(http_request *this);

#endif
