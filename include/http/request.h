#pragma once

#include "body.h"
#include "version.h"
#include "results.h"

#include <stddef.h>

struct http_request;
typedef struct http_request http_request;

DECLARE_RESULT_TYPE(http_request *, HTTPRequestResult);


HTTPRequestResult http_request_new(void);

/**
 * Creates a new http_request from an array of bytes
 * validates basic request format but does not enforce
 * multiple RFC validations
 *
 * @param data  Byte array
 * @param len   Length of byte array
 *
 * @returns HTTPRequestResult. Must unwrap to get http_request
 */
ErrorMessage http_request_parse(http_request* this, const char *data, size_t len);

/**
 * Clean and delete http_request pointer
 *
 * @param this  Request to be deleted
 */
void http_request_delete(http_request *this);

/**
 * http_request.method getter
 *
 * @param this  Request
 *
 * @returns StringResult. Must unwrap to get method string
 */
StringResult http_request_Method(http_request *this);

/**
 * http_request.uri getter
 *
 * @param this  Request
 *
 * @returns StringResult. Must unwrap to get uri string
 */
StringResult http_request_Uri(http_request *this);

/**
 * http_request.version getter
 *
 * @param this Request
 *
 * @returns HTTPVersionResult. Must unwrap to get http_version
 */
HTTPVersionResult http_request_Version(http_request *this);

/**
 * http_request.body getter
 *
 * @param this Request
 *
 * @returns HTTPBodyResult. Must unwrap to get http_request_body
 */
HTTPBodyResult http_request_Body(http_request *this);

/**
 * Adds a kv pair to http_request.header
 *
 * @param this          Request
 * @param headerKey     Key for storage
 * @param headerValue   Value to store
 *
 * @returns Error string. Null if no error
 */
const char *http_request_HeaderSetValue(http_request *this,
                                        const char *headerKey,
                                        const char *headerValue);

/**
 * Retrieves a kv pair from http_request.header
 *
 * @param this          Request
 * @param headerKey     Key to lookup
 *
 * @returns StringResult. Must unwrap to get value string.
 */
ConstStringResult http_request_HeaderGetValue(http_request *this,
                                         const char *headerKey);

/**
 * Get an array with all the header keys in the http_request
 *
 * @param this          Request
 * @param keys_length   size_t address to store length of keys array
 *
 * @returns StringArrResult. Must unwrap to get key array. Must free arr after
 * use
 */
ConstStringArrResult http_request_HeaderKeys(http_request *this, size_t *keys_length);

/**
 * Whether or not this.header contains the key in 'headerKey'
 *
 * @param this      Request
 * @param headerKey Key to look for in this.header
 *
 * @returns bool    True if 'headerKey' is found else False
 */
BoolResult http_request_HeaderContains(http_request *this, const char *headerKey);


static inline void cleanup_http_request(http_request** p) {
    http_request_delete(*p);
}
