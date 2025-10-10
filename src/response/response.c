#include "response_internal.h"
#include <http/response.h>

#include "response/response_codes.h"
#include "sds.h"
#include <stdlib.h>

http_response*   http_response_new() {
    http_response* new_response = malloc(sizeof(http_response));

    new_response->status_code = HTTP_NO_STATUS;
    new_response->reason_phrase = sdsnew("");
    new_response->version = "HTTP/1.1";
    new_response->header = NULL;
    new_response->body.length = 0;
    new_response->body.data = NULL;

    return new_response;
}

char*       http_response_bytes(http_response* this);

void        http_response_delete(http_response* this);
