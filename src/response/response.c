#include "logger/logger.h"
#include "map/map.h"
#include "response_internal.h"
#include <http/response.h>

#include "response/response_codes.h"
#include "sds.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

http_response *http_response_new(void) {
    http_response *new_response = malloc(sizeof(http_response));

    new_response->status_code = HTTP_NO_STATUS;
    new_response->reason_phrase = NULL;
    new_response->version.major = 1;
    new_response->version.minor = 1;
    new_response->header = NULL;
    new_response->body.length = 0;
    new_response->body.data = NULL;

    return new_response;
}

sds http_response_bytes(http_response *this) {
    sds response_string = sdsnew("");
    if (this->version.major > 1) {
        response_string = sdscatprintf(response_string, "HTTP/%d %d %s\r\n",
                                       this->version.major, this->status_code,
                                       this->reason_phrase);
    } else {
        response_string = sdscatprintf(response_string, "HTTP/%d.%d %d %s\r\n",
                                       this->version.major, this->version.minor,
                                       this->status_code, this->reason_phrase);
    }
    if (this->header) {
        size_t keys_length;
        const char **headerKeys = http_response_HeaderKeys(this, &keys_length);
        for (size_t i = 0; i < keys_length; i++) {
            const char *key = headerKeys[i];
            const char *value = http_response_HeaderGetValue(this, key);

            response_string =
                sdscatprintf(response_string, "%s: %s\r\n", key, value);
        }
        free(headerKeys);
    }

    response_string = sdscat(response_string, "\r\n");

    if (this->body.data && this->body.length > 0) {
        if (!http_response_HeaderContains(this, "content-length")) {
            LOG_ERROR("Invalid response: key 'content-length' must be set if "
                      "response has body.");
            sdsfree(response_string);
            return NULL;
        }
        response_string = sdscatlen(
            response_string, (const char *)this->body.data, this->body.length);
    }

    return response_string;
}

void http_response_delete(http_response *this) {
    if (this) {
        if (this->reason_phrase)
            sdsfree(this->reason_phrase);
        if (this->header)
            map_delete(this->header);
        if (this->body.length > 0 || this->body.data) {
            free(this->body.data);
        }
        free(this);
    }
}

uint16_t http_response_StatusCode(http_response *this) {
    return this->status_code;
}

void http_response_SetStatusCode(http_response *this, uint16_t code) {
    this->status_code = code;
}

const char *http_response_ReasonPhrase(http_response *this) {
    return this->reason_phrase;
}

void http_response_SetReasonPhrase(http_response *this,
                                   const char *reason_phrase) {
    if (!this->reason_phrase)
        this->reason_phrase = sdsnew(reason_phrase);
    else
        this->reason_phrase = sdscpy(this->reason_phrase, reason_phrase);
}

http_version http_response_Version(http_response *this) {
    return this->version;
}

void http_response_SetVersion(http_response *this, uint8_t major,
                              uint8_t minor) {
    this->version.major = major;
    this->version.minor = minor;
}

void http_response_HeaderSetValue(http_response *this, const char *headerKey,
                                  const char *headerValue) {
    if (!this->header)
        this->header = map_new();

    if (!isStringSafe(headerKey, strlen(headerKey))) {
        LOG_ERROR("CRLF sequence rejected in header key");
        return;
    }
    if (!isStringSafe(headerValue, strlen(headerKey))) {
        LOG_ERROR("CRLF sequence rejected in header value");
        return;
    }

    sds lowerCaseKey = sdsnew(headerKey);
    sdstolower(lowerCaseKey);

    sds cleanValue = sdsnew(headerValue);
    cleanValue = sdstrim(cleanValue, " ");

    this->header = map_set(this->header, lowerCaseKey, cleanValue);
    sdsfree(cleanValue);
    sdsfree(lowerCaseKey);
}

const char *http_response_HeaderGetValue(http_response *this,
                                         const char *headerKey) {
    return map_get(this->header, headerKey);
}

const char **http_response_HeaderKeys(http_response *this,
                                      size_t *keys_length) {
    if (!this->header)
        return NULL;
    return map_keys(this->header, keys_length);
}

bool http_response_HeaderContains(http_response *this, const char *headerKey) {
    const char **headerKeys = NULL;
    size_t headerKeysLength = 0;

    headerKeys = map_keys(this->header, &headerKeysLength);

    bool contains = str_arr_contains(headerKeys, headerKey, headerKeysLength);
    free(headerKeys);

    return contains;
}

int http_response_SetBody(http_response *this, void *data, size_t length) {
    if (!data) {
        return 1;
    }
    if (this->body.length != 0 || this->body.data) {
        free(this->body.data);
    }
    this->body.length = length;
    this->body.data = malloc(this->body.length);

    memcpy(this->body.data, data, length);

    return 0;
}

http_body *http_response_GetBody(http_response *this) { return &this->body; }

bool isStringSafe(const char *string, size_t length) {
    for (size_t i = 0; i < length; i++) {
        switch (string[i]) {
        case '\r':
            return false;
            break;
        case '\n':
            return false;
            break;
        default:
            continue;
        }
    }

    return true;
}
