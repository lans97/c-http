#include "request_internal.h"
#include <http/request.h>

#include "http/body.h"
#include "http/results.h"
#include "http/version.h"

#include "logger/logger.h"
#include "map/map.h"
#include "sds.h"

#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DEFINE_RESULT_TYPE(http_request*, HTTPRequestResult);

HTTPRequestResult http_request_fromBytes(const char *data, size_t len) {
    http_request* req __attribute__((cleanup(cleanup_http_request))) = malloc(sizeof(http_request));

    const char *header_end = NULL;
    const char *line_end = NULL;

    size_t key_count;

    header_end = strstr(data, "\r\n\r\n");
    if (header_end == NULL)
        return HTTPRequestResult_Error("Malformed request: Missing header-body separator.");

    size_t headers_len = header_end - data;

    if (!req)
        return HTTPRequestResult_Error("Not enough memory");

    req->method = NULL;
    req->uri = NULL;
    req->version.major = UINT8_MAX;
    req->version.minor = UINT8_MAX;
    req->header = NULL;
    req->body.length = 0;
    req->body.data = NULL;

    line_end = strstr(data, "\r\n");
    if (line_end == NULL || line_end > header_end) {
        return HTTPRequestResult_Error("Malformed request: Cannot find end of request line.");
    }

    if (parse_request_line(req, data, line_end - data) != 0) {
        // TODO: Handle Err
    }

    if (parse_headers(req, line_end + 2, headers_len - (line_end - data) - 2) !=
        0) {
        // TODO: Handle Err
    }

    const char *body_start = header_end + strlen("\r\n\r\n");
    size_t body_len = len - (body_start - data);

    if (body_len > 0) {
        http_body_initWithBody(&req->body, (void*) body_start, body_len);
    } else {
        req->body.data = NULL;
    }

    if (req->header != NULL) {
        const char** keys;
        keys = map_keys(req->header, &key_count);
        if (str_arr_contains(keys, "content-length", key_count)) {
            char *endptr;
            size_t expected_len =
                strtoul(map_get(req->header, "content-length"), &endptr, 10);
            if (expected_len != req->body.length) {
                LOG_WARNING("Content-Length header (%zu) does not match actual "
                            "body lenght (%zu). Request may be invalid.",
                            expected_len, req->body.length);
                const char* m = "Content length does not match";
                free(keys);
                return HTTPRequestResult_Error("Invalid header value 'Content-Length'");
            }
        }
    }

    HTTPRequestResult res = { .Ok = true, .Value = req};
    req = NULL;
    return res;
}

void http_request_delete(http_request *this) {
    if (this) {
        if (this->method)
            sdsfree(this->method);
        if (this->uri)
            sdsfree(this->uri);
        if (this->header)
            map_delete(this->header);
        if (this->body.data)
            free(this->body.data);
        free(this);
    }
}

ErrorMessage parse_request_line(http_request *req, const char *data, size_t len) {
    size_t sp1 = 0, sp2 = 0;

    for (size_t i = 0; i < len && (!sp1 || !sp2); i++) {
        if (!sp1 && data[i] == ' ')
            sp1 = i;
        else if (sp1 && !sp2 && data[i] == ' ')
            sp2 = i;
    }

    if (!sp1 || !sp2 || sp2 == sp1 + 1)
        return "Malformed request line: Incorrect number of spaces or zero "
               "length component.";

    size_t method_start = 0;
    size_t method_len = sp1;

    size_t uri_start = sp1 + 1;
    size_t uri_len = sp2 - sp1 - 1;

    size_t version_start = sp2 + 1;
    size_t version_len = len - sp2 - 1;

    if (version_len == 0 || (version_start + version_len != len))
        return "Malformed requiest line: Missing HTTP version or junk "
                  "characters at the end.";

    req->method = sdsnewlen(data + method_start, method_len);
    req->uri = sdsnewlen(data + uri_start, uri_len);

    char version_str[10];
    if (version_len >= 9)
        return "Malformed request line: version too long";

    memcpy(version_str, data + version_start, version_len);
    version_str[version_len] = 0;

    int items_read = sscanf(version_str, "HTTP/%" SCNu8 ".%" SCNu8,
                            &req->version.major, &req->version.minor);

    if (items_read == 0 || items_read > 2)
        return "Malformed request line: incorrect version format.";

    if (items_read == 1) {
        if (req->version.major > 3)
            return "Malformed request line: invalid HTTP version.";
        else
            req->version.minor = 0;
    }

    if (items_read == 2)
        if (!http_version_isValid(&req->version))
            return "Malformed request line: invalid HTTP version.";

    if (!req->method || !req->uri)
        return "Failed to allocate memory for request line components.";

    return NULL;
}

ErrorMessage parse_headers(http_request *req, const char *data, size_t len) {
    const char *cur_line = data;
    const char *block_end = data + len;
    const char *line_end = NULL;
    size_t line_len = 0;

    if (len == 0)
        return 0;

    while (cur_line < block_end) {
        line_end = strstr(cur_line, "\r\n");
        if (line_end == NULL)
            return "Malformed header block: unterminated line.";

        if (line_end > block_end)
            return "Malformed header block: delimiter extends into body.";

        line_len = line_end - cur_line;

        const char* err = parse_single_header(req, cur_line, line_len);
        if (err != NULL)
            return err;

        cur_line = line_end + 2;
    }

    return NULL;
}

ErrorMessage parse_single_header(http_request *req, const char *line, size_t len) {
    const char *colon = (const char *)memchr(line, ':', len);

    if (!colon || colon == line)
        return "Malformed request: header line missing colon [:] or is empty.";

    size_t key_len = colon - line;

    sds key = sdsnewlen(line, key_len);
    if (!key)
        return "Failed to allocate memory for new key string";

    key = sdstrim(key, " ");
    if (sdslen(key) == 0) {
        sdsfree(key);
        return "Malformed request: Invalid key with only whitespace.";
    }

    sdstolower(key);

    const char *value_start = colon + 1;
    size_t value_len = len - (value_start - line);

    sds value = sdsnewlen(value_start, value_len);
    if (!value)
        return "Failed to allocate memory for new value string";
    value = sdstrim(value, " ");

    if (!req->header)
        req->header = map_new();

    req->header = map_set(req->header, key, value);

    sdsfree(key);
    sdsfree(value);

    return NULL;
}

StringResult http_request_Method(http_request *this) { return StringResult_Ok(this->method); }

StringResult http_request_Uri(http_request *this) { return StringResult_Ok(this->uri); }

HTTPVersionResult http_request_Version(http_request *this) {
    return HTTPVersionResult_Ok(&this->version);
}

HTTPBodyResult http_request_Body(http_request *this) {
    return HTTPBodyResult_Ok(&this->body);
}

const char *http_request_HeaderSetValue(http_request *this,
                                        const char *headerKey,
                                        const char *headerValue) {
    this->header = map_set(this->header, headerKey, headerValue);
    if (!this->header)
        return "Map error: Something went wrong!";

    return NULL;
}

ConstStringResult http_request_HeaderGetValue(http_request *this,
                                        const char *headerKey) {
    return ConstStringResult_Ok(map_get(this->header, headerKey));
}

ConstStringArrResult http_request_HeaderKeys(http_request *this, size_t *keys_length) {
    return ConstStringArrResult_Ok(map_keys(this->header, keys_length));
}

BoolResult http_request_HeaderContains(http_request *this, const char *headerKey) {
    const char** headerKeys = NULL;
    size_t headerKeysLength = 0;

    headerKeys = map_keys(this->header, &headerKeysLength);
    if (!headerKeys)
        return BoolResult_Error("Failed to get header map keys.");

    bool contains = str_arr_contains(headerKeys, headerKey, headerKeysLength);
    free(headerKeys);

    return BoolResult_Ok(contains);
}

