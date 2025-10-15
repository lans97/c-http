#pragma once

#include "results.h"
#include <stdint.h>

typedef struct http_version {
    uint8_t major;
    uint8_t minor;
} http_version;

DECLARE_RESULT_TYPE(http_version*, HTTPVersionResult);
DEFINE_RESULT_TYPE(http_version*, HTTPVersionResult);

bool http_version_isValid(http_version* this);
