#pragma once

#include <sds.h>
#include <stddef.h>

typedef struct map_pair map_pair;

struct map_pair {
    sds key;
    sds value;

    map_pair* p_next;
    map_pair* p_prev;

    map_pair* next_entry;
    map_pair* prev_entry;
};

/**
 * Free map_pair and contents
 */
void map_pair_delete(map_pair* this);

typedef struct map {
    size_t      size;
    size_t      capacity;
    map_pair*   entries;
    map_pair*   data[];
} map;

/**
 * Create new map with default capacity
 *
 * @returns new malloced map
*/
map*            map_new();

/**
 * Insert key value pair into map
 *
 * @returns new pointer to this map
 *
 * @code
 * map* mymap = map_new();
 * mymap = map_set(mymap, "key", "value");
 * @endcode
 */
map*            map_set(map* this, const char* key, const char* value);

/**
 * Returns a pointer to the value string associated with key
 * or null if key is not found
 */
const char*     map_get(map* this, const char* key);

/**
 * Remove entry and return malloced map_pair
 * Must free pair after use
 */
map_pair*       map_remove_pair(map* this, const char* key);

/**
 * Remove entry and return malloced string value
 * Must free string after use
 */
char*           map_remove_value(map* this, const char* key);

/**
 * Frees given map and contents
 */
void            map_delete(map* this);

/**
 * Returns a malloced array of all the map's used keys
 */
const char**    map_keys(map* this, size_t *keys_len);

map*            map_resize(map* this, size_t capacity);
map*            map_grow(map* this);

size_t          map_hash(const char* s);

/**
 * Returns whether the arr contains a string equal to s
 * All strings must be safe C strings \0 terminate
 */
bool str_arr_contains(const char** arr, const char* s, size_t arr_len);

/**
 * Returns first prime number greater than n
 */
size_t next_prime(size_t n);

