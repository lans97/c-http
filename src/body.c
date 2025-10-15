#include "http/body.h"
#include "http/results.h"

#include <stdlib.h>
#include <string.h>

__attribute__((malloc))
http_body *http_body_new(void) {
    http_body* new_body = malloc(sizeof(http_body));
    new_body->data = NULL;
    new_body->length = 0;
    return new_body;
}

__attribute__((malloc))
http_body *http_body_newWithBody(void *data, size_t length) {
    http_body* new_body = malloc(sizeof(http_body));
    new_body->data = malloc(length);
    memcpy(new_body->data, data, length);
    new_body->length = length;

    return new_body;
}

void http_body_init(http_body* this) {
    this->data = NULL;
    this->length = 0;
}

ErrorMessage http_body_initWithBody(http_body* this, void *data, size_t length) {
    this->data = malloc(length);
    if (!data)
        return "No more memory.";
    memcpy(this->data, data, length);
    this->length = length;

    return NULL;
}

void http_body_deinit(http_body *this) {
    if (this) {
        if (this->length > 0 || this->data)
            free(this->data);
    }
}

void http_body_delete(http_body *this) {
    if (this) {
        if (this->length > 0 || this->data)
            free(this->data);
        free(this);
    }
}
