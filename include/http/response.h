#ifndef RESPONSE_H
#define RESPONSE_H

#include "http/body.h"
#include "http/utils.h"
#include <stddef.h>
#include <stdint.h>
struct response;
typedef struct http_response http_response;

http_response*  http_response_new(void);

char*           http_response_bytes(http_response* this);

void            http_response_delete(http_response* this);

uint16_t        http_response_StatusCode(http_response* this);
void            http_response_SetStatusCode(http_response* this, uint16_t code);

const char*     http_response_ReasonPhrase(http_response* this);
void            http_response_SetReasonPhrase(http_response* this, const char* reason_phrase);

http_version    http_response_Version(http_response* this);
void            http_response_SetVersion(http_response* this, uint8_t major, uint8_t minor);

void            http_response_HeaderSetValue(http_response* this, const char* headerKey, const char* headerValue);
const char*     http_response_HeaderGetValue(http_response* this, const char* headerKey);

const char**    http_response_HeaderKeys(http_response* this, size_t* keys_length);
bool            http_response_HeaderContains(http_response* this, const char* headerKey);

int             http_response_SetBody(http_response* this, void* data, size_t length);
http_body*      http_response_GetBody(http_response* this);

#endif
