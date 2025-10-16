#include "response_internal.h"
#include "http/response.h"

#include "response/response_codes.h"
#include "http/results.h"
#include "http/version.h"

#include "map/map.h"
#include "sds.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DEFINE_RESULT_TYPE(http_response*, HTTPResponseResult);

HTTPResponseResult http_response_new(void) {
    http_response *new_response = malloc(sizeof(http_response));
    if (!new_response)
        return HTTPResponseResult_Error("Failed to allocate memory");

    new_response->status_code = HTTP_NO_STATUS;
    new_response->reason_phrase = NULL;
    new_response->version.major = 1;
    new_response->version.minor = 1;
    new_response->header = NULL;
    new_response->body.length = 0;
    new_response->body.data = NULL;

    return HTTPResponseResult_Ok(new_response);
}

StringResult http_response_bytes(http_response *this) {
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

        ConstStringArrResult res = http_response_HeaderKeys(this, &keys_length);
        if (!res.Ok)
            return StringResult_Error(res.Err);

        const char **headerKeys = res.Value;
        for (size_t i = 0; i < keys_length; i++) {
            const char *key = headerKeys[i];
            ConstStringResult res = http_response_HeaderGetValue(this, key);
            if (!res.Ok)
                return StringResult_Error(res.Err);
            const char *value = res.Value;

            response_string =
                sdscatprintf(response_string, "%s: %s\r\n", key, value);
        }
        free(headerKeys);
    }

    response_string = sdscat(response_string, "\r\n");

    if (this->body.data && this->body.length > 0) {
        BoolResult res = http_response_HeaderContains(this, "content-length");
        if (!res.Ok)
            return StringResult_Error(res.Err);
        if (!res.Value) {
            sdsfree(response_string);
            return StringResult_Error("Invalid response: key 'content-length' must be set if "
                      "response has body.");
        }
        response_string = sdscatlen(
            response_string, (const char *)this->body.data, this->body.length);
    }

    return StringResult_Ok(response_string);
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

UInt16Result http_response_StatusCode(http_response *this) {
    if (!this)
        return UInt16Result_Error("This is null");
    return UInt16Result_Ok(this->status_code);
}

ErrorMessage http_response_SetStatusCode(http_response *this, uint16_t code) {
    if (!this)
        return "This is null";

    this->status_code = code;
    return NULL;
}

ConstStringResult http_response_ReasonPhrase(http_response *this) {
    if (!this)
        return ConstStringResult_Error("This is null");
    return ConstStringResult_Ok(this->reason_phrase);
}

ErrorMessage http_response_SetReasonPhrase(http_response *this,
                                   const char *reason_phrase) {
    if (!this)
        return "This is null";
    if (!this->reason_phrase)
        this->reason_phrase = sdsnew(reason_phrase);
    else
        this->reason_phrase = sdscpy(this->reason_phrase, reason_phrase);

    return NULL;
}

HTTPVersionResult http_response_Version(http_response *this) {
    if (!this)
        HTTPVersionResult_Error("This is null");
    return HTTPVersionResult_Ok(&this->version);
}

ErrorMessage http_response_SetVersion(http_response *this, uint8_t major,
                              uint8_t minor) {
    if (!this)
        return "This is null";
    this->version.major = major;
    this->version.minor = minor;
    return NULL;
}

ErrorMessage http_response_HeaderSetValue(http_response *this, const char *headerKey,
                                  const char *headerValue) {
    if (!this)
        return "This is null";
    if (!this->header)
        this->header = map_new();

    if (!isStringSafe(headerKey, strlen(headerKey))) {
        return "CRLF sequence rejected in header key";
    }
    if (!isStringSafe(headerValue, strlen(headerKey))) {
        return "CRLF sequence rejected in header value";
    }

    sds lowerCaseKey = sdsnew(headerKey);
    sdstolower(lowerCaseKey);

    sds cleanValue = sdsnew(headerValue);
    cleanValue = sdstrim(cleanValue, " ");

    this->header = map_set(this->header, lowerCaseKey, cleanValue);
    sdsfree(cleanValue);
    sdsfree(lowerCaseKey);

    return NULL;
}

ConstStringResult http_response_HeaderGetValue(http_response *this,
                                         const char *headerKey) {
    if (!this)
        return ConstStringResult_Error("This is null");
    return ConstStringResult_Ok(map_get(this->header, headerKey));
}

ConstStringArrResult http_response_HeaderKeys(http_response *this,
                                      size_t *keys_length) {
    if (!this)
        return ConstStringArrResult_Error("This is null");
    if (!this->header)
        return ConstStringArrResult_Error("Header is null");
    return ConstStringArrResult_Ok(map_keys(this->header, keys_length));
}

BoolResult http_response_HeaderContains(http_response *this, const char *headerKey) {
    if (!this)
        return BoolResult_Error("This is null");
    const char **headerKeys = NULL;
    size_t headerKeysLength = 0;

    headerKeys = map_keys(this->header, &headerKeysLength);
    if (!headerKey)
        return BoolResult_Ok(false);

    bool contains = str_arr_contains(headerKeys, headerKey, headerKeysLength);
    free(headerKeys);

    return BoolResult_Ok(contains);
}

ErrorMessage http_response_SetBody(http_response *this, void *data, size_t length) {
    if (!this)
        return "This is null";
    if (!data)
        return "Data is null";
    if (this->body.length != 0 || this->body.data) {
        free(this->body.data);
    }
    this->body.length = length;
    this->body.data = malloc(this->body.length);
    if (!this->body.data)
        return "Out of memory";

    memcpy(this->body.data, data, length);

    return NULL;
}

HTTPBodyResult http_response_GetBody(http_response *this) {
    if (!this)
        return HTTPBodyResult_Error("This is null");
    return HTTPBodyResult_Ok(&this->body);
}
