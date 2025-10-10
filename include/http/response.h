#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdint.h>
struct response;
typedef struct http_response http_response;

http_response*  http_response_new();

char*           http_response_bytes(http_response* this);

void            http_response_delete(http_response* this);

uint16_t        http_response_StatusCode(http_response* this);
const char*     http_response_ReasonPhrase(http_response* this);
const char*     http_response_Version(http_response* this);
int             http_response_HeaderSetValue(http_response* this, const char* headerKey, const char* headerValue);
const char*     http_response_HeaderGetValue(http_response* this, const char* headerKey);

const char**    http_response_HeaderKeys(http_response* this, const char* headerKey);
bool            http_response_HeaderContains(http_response* this, const char* headerKey);

#endif
