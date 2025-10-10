#include "request_internal.h"
#include <http/request.h>

#include "logger/logger.h"
#include "map/map.h"
#include "sds.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

http_request* http_request_fromBytes(const char* data, size_t len) {
    http_request* req = NULL;

    const char* header_end = NULL;
    const char* line_end = NULL;

    size_t key_count;
    const char** keys = NULL;

    header_end = strstr(data, "\r\n\r\n");
    if (header_end == NULL) {
        LOG_ERROR("Malformed request: Missing header-body separator.");
        goto cleanup;
    }
    size_t headers_len = header_end - data;

    req = malloc(sizeof(http_request));
    if (!req) goto cleanup;

    req->method = NULL;
    req->uri = NULL;
    req->version = NULL;
    req->header = NULL;
    req->body.length = 0;
    req->body.data = NULL;

    line_end = strstr(data, "\r\n");
    if (line_end == NULL || line_end > header_end) {
        LOG_ERROR("Malformed request: Cannot find end of request line.");
        goto cleanup;
    }

    if (parse_request_line(req, data, line_end - data) != 0) goto cleanup;

    if (parse_headers(req, line_end + 2, headers_len - (line_end - data) - 2) != 0)
        goto cleanup;

    const char* body_start = header_end + strlen("\r\n\r\n");
    size_t body_len = len - (body_start - data);

    req->body.length = body_len;
    if (body_len > 0) {
        req->body.data = malloc(body_len);
        if (!req->body.data) goto cleanup;
        memcpy(req->body.data, body_start, body_len);
    } else {
        req->body.data = NULL;
    }

    if (req->header != NULL) {
        keys = map_keys(req->header, &key_count);
        if (str_arr_contains(keys, "content-length", key_count)) {
            char* endptr;
            size_t expected_len = strtoul(map_get(req->header, "content-length"), &endptr, 10);
            if (expected_len != req->body.length) {
                LOG_WARNING("Content-Length header (%zu) does not match actual body lenght (%zu). Request may be invalid.", expected_len, req->body.length);
                goto cleanup;
            }
        }
    }

    if (keys) free(keys);
    return req;

cleanup:
    if (req) {
        http_request_delete(req);
    }
    if (keys) free(keys);

    return NULL;
}

void http_request_delete(http_request* this) {
    if (this) {
        if (this->method) sdsfree(this->method);
        if (this->uri) sdsfree(this->uri);
        if (this->version) sdsfree(this->version);
        if (this->header) map_delete(this->header);
        if (this->body.data) free(this->body.data);
        free(this);
    } else {
        LOG_WARNING("this http_request is null");
    }
}

void free_sdsarr(sds* arr, int arrlen) {
    for (int i = 0; i < arrlen; i++) {
        sdsfree(arr[i]);
    }
}

int parse_request_line(http_request* req, const char* data, size_t len) {
    size_t sp1 = 0, sp2 = 0;

    for (size_t i = 0; i < len && (!sp1 || !sp2); i++) {
        if (!sp1 && data[i] == ' ')
            sp1 = i;
        else if (sp1 && !sp2 && data[i] == ' ')
            sp2 = i;
    }

    if (!sp1 || !sp2 || sp2 == sp1+1) {
        LOG_ERROR(
            "Malformed request line: Incorrect number of spaces or zero length component."
        );
        return 1;
    }

    size_t method_start = 0;
    size_t method_len = sp1;

    size_t uri_start = sp1+1;
    size_t uri_len = sp2-sp1-1;

    size_t version_start = sp2+1;
    size_t version_len = len-sp2-1;

    if (version_len == 0 || (version_start + version_len != len)) {
        LOG_ERROR("Malformed requiest line: Missing HTTP version or junk characters at the end.");
        return 1;
    }

    req->method = sdsnewlen(data + method_start, method_len);
    req->uri = sdsnewlen(data + uri_start, uri_len);
    req->version = sdsnewlen(data + version_start, version_len);

    if (!req->method || !req->uri || !req->version) {
        LOG_ERROR("Failed to allocate memory for request line components.");
        return 1;
    }

    return 0;
}

int parse_headers(http_request* req, const char* data, size_t len) {
    const char* cur_line = data;
    const char* block_end = data + len;
    const char* line_end = NULL;
    size_t line_len = 0;

    if (len == 0)
        return 0;

    while (cur_line < block_end) {
        line_end = strstr(cur_line, "\r\n");
        if (line_end == NULL) {
            LOG_ERROR("Malformed header block: unterminated line.");
            return 1;
        }

        if (line_end > block_end) {
            LOG_ERROR("Malformed header block: delimiter extends into body.");
            return 1;
        }

        line_len = line_end - cur_line;

        if (parse_single_header(req, cur_line, line_len) != 0)
            return 1;

        cur_line = line_end + 2;
    }

    return 0;
}

int parse_single_header(http_request* req, const char* line, size_t len) {
    const char* colon = (const char*) memchr(line, ':', len);

    if (!colon || colon == line) {
        LOG_ERROR("Malformed request: header line missing colon [:] or is empty.");
        return 1;
    }

    size_t key_len = colon - line;

    sds key = sdsnewlen(line, key_len);
    if (!key) {
        LOG_ERROR("Failed to allocate memory for new key string");
        return 1;
    }

    key = sdstrim(key, " ");
    if (sdslen(key) == 0) {
        sdsfree(key);
        return 1;
    }

    sdstolower(key);

    const char* value_start = colon + 1;
    size_t value_len = len - (value_start - line);

    sds value = sdsnewlen(value_start, value_len);
    if (!value) {
        LOG_ERROR("Failed to allocate memory for new value string");
        return 1;
    }
    value = sdstrim(value, " ");

    if (!req->header)
        req->header = map_new();

    req->header = map_set(req->header, key, value);

    sdsfree(key);
    sdsfree(value);

    return 0;
}
