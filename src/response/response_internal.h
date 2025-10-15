#pragma once

#include "http/body.h"
#include "http/utils.h"
#include <map/map.h>
#include <stdint.h>

struct http_response {
    uint16_t            status_code;
    sds                 reason_phrase;
    http_version        version;
    map*                header;
    http_body           body;
};

bool isStringSafe(const char* string, size_t length);

