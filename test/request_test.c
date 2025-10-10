#include "http/request.h"
#include "map/map.h"
#include "request/request_internal.h"
#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <unity_internals.h>

http_request *req;

void setUp(void) {
}

void tearDown(void) { http_request_delete(req); }

void test_http_request_fromBytes_Success(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: MyTestClient/1.0\r\n"
        "Content-Length: 28\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-CUSTOM-HEADER:    with leading space\r\n"
        "\r\n"
        "{\"name\": \"Alice\", \"age\": 30}";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_EQUAL_STRING("POST", req->method);
    TEST_ASSERT_EQUAL_STRING("/api/v1/users", req->uri);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1", req->version);
    TEST_ASSERT_EQUAL_STRING("example.com", map_get(req->header, "host"));
    TEST_ASSERT_EQUAL_STRING("MyTestClient/1.0",
                             map_get(req->header, "user-agent"));
    TEST_ASSERT_EQUAL_STRING("28", map_get(req->header, "content-length"));
    TEST_ASSERT_EQUAL_STRING("application/json; charset=utf-8",
                             map_get(req->header, "content-type"));
    TEST_ASSERT_EQUAL_STRING("with leading space",
                             map_get(req->header, "x-custom-header"));
    TEST_ASSERT_EQUAL_UINT(28, req->body.length);
    TEST_ASSERT_NOT_NULL(req->body.data);
}

void test_http_request_fromBytes_WrongContentLength_Fail(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: MyTestClient/1.0\r\n"
        "Content-Length: 35\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-CUSTOM-HEADER:    with leading space\r\n"
        "\r\n"
        "{\"name\": \"Alice\", \"age\": 30}";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NULL(req);
}

void test_http_request_fromBytes_EmptyHeader_Success(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n\r\n";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_EQUAL_STRING("POST", req->method);
    TEST_ASSERT_EQUAL_STRING("/api/v1/users", req->uri);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1", req->version);

    TEST_ASSERT_NULL(req->header);

    TEST_ASSERT_EQUAL_UINT(0, req->body.length);
    TEST_ASSERT_NULL(req->body.data);
}

void test_http_request_fromBytes_IncorrectRequestLine_Fail(void) {
    const char *exampleRequest =
        "POST /api/v1/usersJUNKHTTP/1.1\r\n\r\n";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NULL(req);
}

void test_http_request_fromBytes_MissingSpace_Fail(void) {
    const char *exampleRequest =
        "POST /api/v1/usersHTTP/1.1\r\n\r\n";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NULL(req);
}

void test_http_request_fromBytes_EmptyHeaderKey_Fail(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        ": value\r\n"
        "\r\n";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NULL(req);
}

void test_http_request_fromBytes_WhitespaceHeaderKey_Fail(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        " : value\r\n"
        "\r\n";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NULL(req);
}

void test_http_request_fromBytes_EmptyHeaderValue_Success(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "X-No-Value:\r\n"
        "\r\n";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NOT_NULL(req);
    TEST_ASSERT_EQUAL_STRING("", map_get(req->header, "x-no-value"));
}

void test_http_request_fromBytes_Body_Success(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: MyTestClient/1.0\r\n"
        "Content-Length: 28\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-CUSTOM-HEADER:    with leading space\r\n"
        "\r\n"
        "{\"name\": \"Alice\", \"age\": 30}";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_NOT_NULL(req->body.data);
    TEST_ASSERT_EQUAL_STRING("{\"name\": \"Alice\", \"age\": 30}", req->body.data);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_http_request_fromBytes_Success);
    RUN_TEST(test_http_request_fromBytes_WrongContentLength_Fail);
    RUN_TEST(test_http_request_fromBytes_EmptyHeader_Success);
    RUN_TEST(test_http_request_fromBytes_IncorrectRequestLine_Fail);
    RUN_TEST(test_http_request_fromBytes_MissingSpace_Fail);
    RUN_TEST(test_http_request_fromBytes_EmptyHeaderKey_Fail);
    RUN_TEST(test_http_request_fromBytes_WhitespaceHeaderKey_Fail);
    RUN_TEST(test_http_request_fromBytes_EmptyHeaderValue_Success);
    RUN_TEST(test_http_request_fromBytes_Body_Success);

    return UNITY_END();
}
