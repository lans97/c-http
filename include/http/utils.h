#pragma once
#include <assert.h>
#include <stdint.h>
#include <sds.h>

void free_sdsarr(sds* arr, int arrlen);
bool isStringSafe(const char* string, size_t length);
