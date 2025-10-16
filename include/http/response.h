#pragma once

#include "body.h"
#include "http/results.h"
#include "http/version.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
struct response;
typedef struct http_response http_response;

DECLARE_RESULT_TYPE(http_response*, HTTPResponseResult);

HTTPResponseResult      http_response_new(void);

StringResult            http_response_bytes(http_response* this);

void                    http_response_delete(http_response* this);

UInt16Result            http_response_StatusCode(http_response* this);
ErrorMessage            http_response_SetStatusCode(http_response* this, uint16_t code);

ConstStringResult       http_response_ReasonPhrase(http_response* this);
ErrorMessage            http_response_SetReasonPhrase(http_response* this, const char* reason_phrase);

HTTPVersionResult       http_response_Version(http_response* this);
ErrorMessage            http_response_SetVersion(http_response* this, uint8_t major, uint8_t minor);

ErrorMessage            http_response_HeaderSetValue(http_response* this, const char* headerKey, const char* headerValue);
ConstStringResult       http_response_HeaderGetValue(http_response* this, const char* headerKey);

ConstStringArrResult    http_response_HeaderKeys(http_response* this, size_t* keys_length);
BoolResult              http_response_HeaderContains(http_response* this, const char* headerKey);

ErrorMessage            http_response_SetBody(http_response* this, void* data, size_t length);
HTTPBodyResult          http_response_GetBody(http_response* this);

static inline void cleanup_http_response(http_response** p) {
    http_response_delete(*p);
}
