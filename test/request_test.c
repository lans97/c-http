#include "http/request.h"
#include "http/utils.h"
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

    http_version expected_version;
    expected_version.major = 1;
    expected_version.minor = 1;

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_EQUAL_STRING("POST", req->method);
    TEST_ASSERT_EQUAL_STRING("/api/v1/users", req->uri);
    TEST_ASSERT_EQUAL_UINT8(expected_version.major, req->version.major);
    TEST_ASSERT_EQUAL_UINT8(expected_version.minor, req->version.minor);
    TEST_ASSERT_EQUAL_STRING("example.com", http_request_HeaderGetValue(req, "host"));
    TEST_ASSERT_EQUAL_STRING("MyTestClient/1.0",
                             http_request_HeaderGetValue(req, "user-agent"));
    TEST_ASSERT_EQUAL_STRING("28", http_request_HeaderGetValue(req, "content-length"));
    TEST_ASSERT_EQUAL_STRING("application/json; charset=utf-8",
                             http_request_HeaderGetValue(req, "content-type"));
    TEST_ASSERT_EQUAL_STRING("with leading space",
                             http_request_HeaderGetValue(req, "x-custom-header"));
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

    http_version expected_version;
    expected_version.major = 1;
    expected_version.minor = 1;

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_EQUAL_STRING("POST", req->method);
    TEST_ASSERT_EQUAL_STRING("/api/v1/users", req->uri);
    TEST_ASSERT_EQUAL_UINT8(expected_version.major, req->version.major);
    TEST_ASSERT_EQUAL_UINT8(expected_version.minor, req->version.minor);

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
    TEST_ASSERT_EQUAL_STRING("", http_request_HeaderGetValue(req, "x-no-value"));
}

void test_http_request_fromBytes_Body_Success(void) {
    const char *exampleRequest =
        "POST /users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 49\r\n"
        "\r\n"
        "name=FirstName+LastName&email=bsmth%40example.com";

    req = http_request_fromBytes(exampleRequest, strlen(exampleRequest));

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_NOT_NULL(req->body.data);
    TEST_ASSERT_EQUAL_MEMORY("name=FirstName+LastName&email=bsmth%40example.com", req->body.data, req->body.length);
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
