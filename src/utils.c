#include "http/utils.h"

void free_sdsarr(sds *arr, int arrlen) {
    for (int i = 0; i < arrlen; i++) {
        sdsfree(arr[i]);
    }
}

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
