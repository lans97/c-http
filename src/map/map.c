#include "map.h"
#include "logger/logger.h"
#include "sds.h"
#include "wyhash.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAP_DEFAULT_CAPACITY 11

void map_pair_delete(map_pair* this) {
    sdsfree(this->key);
    sdsfree(this->value);

    free(this);
}

map* map_new() {
    size_t capacity = MAP_DEFAULT_CAPACITY;
    size_t data_size = sizeof(map_pair) * capacity;
    size_t total_size = sizeof(map) + data_size;

    map* new_map = malloc(total_size);
    if (new_map == NULL) {
        LOG_ERROR("Failed to allocate memory for new map: %s", strerror(errno));
        return NULL;
    }

    new_map->size = 0;
    new_map->capacity = capacity;
    new_map->entries = NULL;

    //** set array to null pointers
    memset(new_map->data, 0, data_size);

    return new_map;
}

// No resizing just yet
map* map_set(map* this, const char* key, const char* value) {
    if (this == NULL) {
        LOG_WARNING("this map is NULL");
        return NULL;
    }
    size_t idx = map_hash(key) % this->capacity;

    // Determine if key already exists
    size_t keys_len;
    const char** keys = map_keys(this, &keys_len);
    if (str_arr_contains(keys, key, keys_len)) {
        map_pair* cur = this->data[idx];
        while (cur != NULL) {
            if (strcmp(cur->key, key) == 0) {
                sdscpy(cur->value, value);
                return this;
            }
            cur = cur->p_next;
        }
        if (cur == NULL) {
            LOG_ERROR("Unexpected error while setting kv pair: <%s, %s>", key, value);
            return this;
        }
    } else { // new key
        // grow if needed
        this = map_grow(this);
        map_pair* new_pair = malloc(sizeof(map_pair));
        if (new_pair == NULL) {
            LOG_ERROR("Failed to allocate memory for new pair: %s", strerror(errno));
            return this;
        }
        new_pair->key = sdsnew(key);
        new_pair->value = sdsnew(value);

        // Handle bucket insertion
        new_pair->p_next = this->data[idx];
        new_pair->p_prev = NULL;
        if (this->data[idx] != NULL)
            this->data[idx]->p_prev = new_pair;
        this->data[idx] = new_pair;

        // Handle entries insertion
        new_pair->next_entry = this->entries;
        new_pair->prev_entry = NULL;
        if (this->entries != NULL)
            this->entries->prev_entry = new_pair;
        this->entries = new_pair;

        this->size++;
    }

    return this;
}

const char* map_get(map* this, const char* key) {
    if (this == NULL) {
        LOG_WARNING("this map is NULL");
        return NULL;
    }
    if (this->size < 1)
        return NULL;

    size_t idx = map_hash(key) % this->capacity;
    map_pair* cur = this->data[idx];
    while (cur != NULL) {
        if (strcmp(cur->key, key) == 0)
            return cur->value;
        cur = cur->p_next;
    }

    return NULL;
}

map_pair* map_remove_pair(map* this, const char* key) {
    if (this == NULL) {
        LOG_WARNING("this map is NULL");
        return NULL;
    }
    if (this->size < 1)
        return NULL;

    size_t idx = map_hash(key) % this->capacity;
    map_pair* cur = this->data[idx];
    while (cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            // FOUND
            map_pair* n = cur->p_next;
            map_pair* p = cur->p_prev;

            if (p != NULL)
                p->p_next = n;
            else
                this->data[idx] = NULL;
            if (n != NULL)
                n->p_prev = p;

            map_pair* ne = cur->next_entry;
            map_pair* pe = cur->prev_entry;

            if (pe != NULL)
                pe->next_entry = ne;
            else
                this->entries = NULL;
            if (ne != NULL)
                ne->prev_entry = pe;

            this->size--;
            return cur;
        }
        cur = cur->p_next;
    }

    return NULL;
}

char* map_remove_value(map* this, const char* key) {
    if (this == NULL) {
        LOG_WARNING("this map is NULL");
        return NULL;
    }
    if (this->size < 1)
        return NULL;

    size_t idx = map_hash(key) % this->capacity;
    map_pair* cur = this->data[idx];

    char* value = malloc(sizeof(char) * sdslen(cur->value)+1);
    if (value == NULL) {
        LOG_ERROR("Failed to allocate memory for new string: %s", strerror(errno));
        return NULL;
    }
    while (cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            // FOUND
            map_pair* n = cur->p_next;
            map_pair* p = cur->p_prev;

            if (p != NULL)
                p->p_next = n;
            else
                this->data[idx] = NULL;
            if (n != NULL)
                n->p_prev = p;

            map_pair* ne = cur->next_entry;
            map_pair* pe = cur->prev_entry;

            if (pe != NULL)
                pe->next_entry = ne;
            else
                this->entries = NULL;
            if (ne != NULL)
                ne->prev_entry = pe;

            strcpy(value, cur->value);

            map_pair_delete(cur);

            this->size--;
            return value;
        }
        cur = cur->p_next;
    }

    free(value);
    return NULL;
}

void map_delete(map* this) {
    if (this == NULL) {
        LOG_ERROR("this map is null");
        return;
    }

    while (this->entries != NULL) {
        map_pair* temp = this->entries;
        this->entries = temp->next_entry;
        map_pair_delete(temp);
    }

    free(this);
}

const char** map_keys(map* this, size_t *keys_len) {
    const char** keys = malloc(this->size * sizeof(char*));
    if (keys == NULL) {
        LOG_ERROR("Failed to allocate memmory for keys array: %s", strerror(errno));
        return NULL;
    }

    size_t i = 0;
    // Get keys from entries list, not buckets
    map_pair* cur = this->entries;
    while (cur != NULL) {
        // add keys to output array
        keys[i] = cur->key;
        i++;
        cur = cur->next_entry;
    }

    // true length of keys array should equal this->size
    *keys_len = i;
    return keys;
}

map* map_resize(map* this, size_t capacity) {
    size_t data_size = sizeof(map_pair) * capacity;
    size_t total_size = sizeof(map) + data_size;

    map* new_map = malloc(total_size);
    if (new_map == NULL) {
        LOG_ERROR("Failed to allocate memory for new map: %s", strerror(errno));
        return this;
    }

    new_map->size = 0;
    new_map->capacity = capacity;
    new_map->entries = NULL;

    //** set array to null pointers
    memset(new_map->data, 0, data_size);

    map_pair* cur = this->entries;
    while (cur != NULL) {
        // Set ownership to new map without deleting data
        map_pair* next = cur->next_entry;
        size_t idx = map_hash(cur->key) % new_map->capacity;

        cur->p_next = new_map->data[idx];
        cur->p_prev = NULL;
        if (new_map->data[idx] != NULL)
            new_map->data[idx]->p_prev = cur;
        new_map->data[idx] = cur;

        cur->next_entry = new_map->entries;
        cur->prev_entry = NULL;
        if (new_map->entries != NULL)
            new_map->entries->prev_entry = cur;
        new_map->entries = cur;

        cur = next;
        new_map->size++;
    }

    free(this);

    return new_map;
}

map* map_grow(map* this) {
    if (this->size*100 > this->capacity*75) {
        this = map_resize(this, next_prime((this->capacity * 17) / 10));
    }
    return this;
}

size_t map_hash(const char *s) {
    return (size_t) wyhash(s, strlen(s), 0, _wyp);
}

bool str_arr_contains(const char** arr, const char* s, size_t arr_len) {
    for (size_t i = 0; i < arr_len; i++) {
        if (strcmp(arr[i], s) == 0)
            return true;
    }

    return false;
}

size_t next_prime(size_t n) {
    if ( n <= 2) return 2;
    if (!(n & 1)) n++;
    while (!is_prime(n))
        n+=2;
    return n;
}
