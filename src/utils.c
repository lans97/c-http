#include "http/utils.h"

void free_sdsarr(sds *arr, int arrlen) {
    for (int i = 0; i < arrlen; i++) {
        sdsfree(arr[i]);
    }
}

