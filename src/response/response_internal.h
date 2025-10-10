#ifndef RESPONSE_INTERNAL_H
#define RESPONSE_INTERNAL_H

#include "body.h"
#include <map/map.h>
#include <stdint.h>

struct http_response {
    uint16_t            status_code;
    sds                 reason_phrase;
    sds                 version;
    map*                header;
    struct http_body    body;
};

#endif
