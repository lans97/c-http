#pragma once

#include "results.h"
#include <stddef.h>

typedef struct http_body {
    void *data;
    size_t length;
} http_body;

/**
 * Allocates new http_body
 *
 * @returns Pointer to new http_body
 */
http_body      *http_body_new(void);

/**
 * Allocates new http_body with data
 * Does a deep copy of data
 *
 * @param data      Pointer to data
 * @param length    Size in bytes of data
 *
 * @returns Pointer to new http_body
 */
http_body      *http_body_newWithBody(void *data, size_t length);

/**
 * Initializes existing http_body
 * Does a deep copy of data
 *
 * @param data      Pointer to data
 * @param length    Size in bytes of data
 */
void            http_body_init(http_body* this);

/**
 * Initializes existing http_body with data
 * Does a deep copy of data
 *
 * @param data      Pointer to data
 * @param length    Size in bytes of data
 *
 * @returns Error message or NULL
 */
ErrorMessage    http_body_initWithBody(http_body* this, void *data, size_t length);

/**
 * Meant to de-initialize stack struct or in struct members
 * Frees inner data only, use http_body_delete to free
 * body obtained with http_body_new or http_body_newWithBody
 *
 * @param this HttpBody to deinit
 */
void            http_body_deinit(http_body *this);

/**
 * Deletes http_body_body and inner data
 *
 * @param this HttpBody to delete
 */
void            http_body_delete(http_body *this);

DECLARE_RESULT_TYPE(http_body *, HTTPBodyResult);
DEFINE_RESULT_TYPE(http_body *, HTTPBodyResult);
