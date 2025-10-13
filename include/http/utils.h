#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stdint.h>
typedef struct http_version {
    uint8_t major;
    uint8_t minor;
} http_version;

bool http_version_isValid(http_version* this);

#endif//HTTP_UTILS_H
