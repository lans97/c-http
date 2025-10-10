#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <unity_internals.h>
#include <map/map.h>

map* m1;

void setUp(void){
    m1 = map_new();
}

void tearDown(void){
    map_delete(m1);
}

void test_MapInsertAndLookup_Success(void) {
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_NOT_NULL(m1);

    const char* value = map_get(m1, "Content-Type");
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_STRING("text/html; charset=utf-8", value);
}

void test_MapLookup_KeyNotFound(void) {
    const char* value = map_get(m1, "key1");
    TEST_ASSERT_NULL(value);
}

void test_MapOverwriteLookup_Success(void) {
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_NOT_NULL(m1);

    m1 = map_set(m1, "Content-Type", "application/json");
    TEST_ASSERT_NOT_NULL(m1);

    const char* value = map_get(m1, "Content-Type");
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_STRING("application/json", value);
}

void test_MapRemoveValueLookup_KeyNotFound(void) {
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_NOT_NULL(m1);

    char* res = map_remove_value(m1, "Content-Type");
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING("text/html; charset=utf-8", res);

    free(res);

    const char* val2 = map_get(m1, "Content-Type");
    TEST_ASSERT_NULL(val2);
}

void test_MapRemovePairLookup_KeyNotFound(void) {
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_NOT_NULL(m1);

    map_pair* res = map_remove_pair(m1, "Content-Type");
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING("text/html; charset=utf-8", res->value);

    map_pair_delete(res);

    const char* res2 = map_get(m1, "Content-Type");
    TEST_ASSERT_NULL(res2);
}

void test_MapIALMultipleEntries_Success(void) {
    m1 = map_set(m1, "Connection", "Keep-Alive");
    TEST_ASSERT_NOT_NULL(m1);
    m1 = map_set(m1, "Content-Encoding", "gzip");
    TEST_ASSERT_NOT_NULL(m1);
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_NOT_NULL(m1);

    const char* value = map_get(m1, "Connection");
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_STRING("Keep-Alive", value);

    value = map_get(m1, "Content-Encoding");
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_STRING("gzip", value);

    value = map_get(m1, "Content-Type");
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_STRING("text/html; charset=utf-8", value);
}

void test_MapSizeTracking_Success(void) {
    TEST_ASSERT_EQUAL_UINT(m1->size, 0);
    m1 = map_set(m1, "Connection", "Keep-Alive");
    TEST_ASSERT_EQUAL_UINT(m1->size, 1);
    m1 = map_set(m1, "Content-Encoding", "gzip");
    TEST_ASSERT_EQUAL_UINT(m1->size, 2);
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_EQUAL_UINT(m1->size, 3);

    free(map_remove_value(m1, "Connection"));
    TEST_ASSERT_EQUAL_UINT(m1->size, 2);
    free(map_remove_value(m1, "Content-Encoding"));
    TEST_ASSERT_EQUAL_UINT(m1->size, 1);
    free(map_remove_value(m1, "Content-Type"));
    TEST_ASSERT_EQUAL_UINT(m1->size, 0);
}

void test_MapOverload_ShouldGrow(void) {
    TEST_ASSERT_EQUAL_UINT(m1->capacity, 11);

    m1 = map_set(m1, "key1", "value1");
    m1 = map_set(m1, "key2", "value2");
    m1 = map_set(m1, "key3", "value3");
    m1 = map_set(m1, "key4", "value4");
    m1 = map_set(m1, "key5", "value5");
    m1 = map_set(m1, "key6", "value6");
    m1 = map_set(m1, "key7", "value7");
    m1 = map_set(m1, "key8", "value8");
    m1 = map_set(m1, "key9", "value9");
    m1 = map_set(m1, "key10", "value10");
    m1 = map_set(m1, "key11", "value11");
    m1 = map_set(m1, "key12", "value12");

    TEST_ASSERT_GREATER_THAN_UINT(11, m1->capacity);
}

void test_MapRemoveValue_KeyNotFound(void) {
    char* res = map_remove_value(m1, "new key");
    TEST_ASSERT_NULL(res);
    free(res);// Good practice
}

void test_MapRemovePair_KeyNotFound(void) {
    map_pair* res = map_remove_pair(m1, "new key");
    TEST_ASSERT_NULL(res);
    free(res);// Good practice
}

void test_MapInsertRemoveInsertLookup_Success(void) {
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_NOT_NULL(m1);

    free(map_remove_value(m1, "Content-Type"));

    m1 = map_set(m1, "Content-Type", "application/json");
    const char* res = map_get(m1, "Content-Type");
    TEST_ASSERT_EQUAL_STRING("application/json", res);
}

void test_MapSizeAfterOverwrite_Success(void) {
    TEST_ASSERT_EQUAL_UINT(m1->size, 0);
    m1 = map_set(m1, "Connection", "Keep-Alive");
    TEST_ASSERT_EQUAL_UINT(m1->size, 1);
    m1 = map_set(m1, "Content-Encoding", "gzip");
    TEST_ASSERT_EQUAL_UINT(m1->size, 2);
    m1 = map_set(m1, "Content-Type", "text/html; charset=utf-8");
    TEST_ASSERT_EQUAL_UINT(m1->size, 3);

    m1 = map_set(m1, "Content-Type", "application/json");
    TEST_ASSERT_EQUAL_UINT(m1->size, 3);

    free(map_remove_value(m1, "Connection"));
    TEST_ASSERT_EQUAL_UINT(m1->size, 2);
    free(map_remove_value(m1, "Content-Encoding"));
    TEST_ASSERT_EQUAL_UINT(m1->size, 1);
    free(map_remove_value(m1, "Content-Type"));
    TEST_ASSERT_EQUAL_UINT(m1->size, 0);
}

void test_MapKeysBeforeGrow_Success(void) {
    m1 = map_set(m1, "key1", "value1");
    m1 = map_set(m1, "key2", "value2");
    m1 = map_set(m1, "key3", "value3");
    m1 = map_set(m1, "key4", "value4");
    m1 = map_set(m1, "key5", "value5");
    m1 = map_set(m1, "key6", "value6");
    m1 = map_set(m1, "key7", "value7");
    m1 = map_set(m1, "key8", "value8");

    size_t keys_len;
    const char** keys = map_keys(m1, &keys_len);

    const char* expected_keys[] = { "key8", "key7", "key6", "key5",
                                    "key4", "key3", "key2", "key1"};

    TEST_ASSERT_EQUAL_STRING_ARRAY(expected_keys, keys, keys_len);
}

void test_MapKeysAfterGrow_Success(void) {
    m1 = map_set(m1, "key1", "value1");
    m1 = map_set(m1, "key2", "value2");
    m1 = map_set(m1, "key3", "value3");
    m1 = map_set(m1, "key4", "value4");
    m1 = map_set(m1, "key5", "value5");
    m1 = map_set(m1, "key6", "value6");
    m1 = map_set(m1, "key7", "value7");
    m1 = map_set(m1, "key8", "value8");
    m1 = map_set(m1, "key9", "value9");
    m1 = map_set(m1, "key10", "value10");
    m1 = map_set(m1, "key11", "value11");
    m1 = map_set(m1, "key12", "value12");


    size_t keys_len;
    const char** keys = map_keys(m1, &keys_len);

    const char* expected_keys[] = { "key12", "key11", "key10", "key9",
                                    "key8",  "key7",  "key6",  "key5",
                                    "key4",  "key3",  "key2",  "key1"};

    // Contains, funcitions
    for (size_t i = 0; i < 12; i++) {
        TEST_ASSERT_TRUE(str_arr_contains(keys, expected_keys[i], keys_len));
    }

    free(keys);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_MapInsertAndLookup_Success);
    RUN_TEST(test_MapLookup_KeyNotFound);
    RUN_TEST(test_MapOverwriteLookup_Success);
    RUN_TEST(test_MapRemoveValueLookup_KeyNotFound);
    RUN_TEST(test_MapRemovePairLookup_KeyNotFound);
    RUN_TEST(test_MapIALMultipleEntries_Success);
    RUN_TEST(test_MapSizeTracking_Success);
    RUN_TEST(test_MapOverload_ShouldGrow);
    RUN_TEST(test_MapRemoveValue_KeyNotFound);
    RUN_TEST(test_MapRemovePair_KeyNotFound);
    RUN_TEST(test_MapInsertRemoveInsertLookup_Success);
    RUN_TEST(test_MapSizeAfterOverwrite_Success);
    RUN_TEST(test_MapKeysBeforeGrow_Success);

    return UNITY_END();
}
