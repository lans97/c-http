#include "http/request.h"
#include "request/request_internal.h"

#include "http/results.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <unity_internals.h>

http_request *req;

void setUp(void) {
    HTTPRequestResult res = http_request_new();
    if (!res.Ok)
        exit(EXIT_FAILURE);
    req = res.Value;
}

void tearDown(void) { http_request_delete(req); }

void test_http_request_parse_Success(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: MyTestClient/1.0\r\n"
        "Content-Length: 28\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-CUSTOM-HEADER:    with leading space\r\n"
        "\r\n"
        "{\"name\": \"Alice\", \"age\": 30}";

    TEST_ASSERT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));

    http_version expected_version;
    expected_version.major = 1;
    expected_version.minor = 1;

    TEST_ASSERT_NOT_NULL(req);

    TEST_ASSERT_EQUAL_STRING("POST", req->method);
    TEST_ASSERT_EQUAL_STRING("/api/v1/users", req->uri);
    TEST_ASSERT_EQUAL_UINT8(expected_version.major, req->version.major);
    TEST_ASSERT_EQUAL_UINT8(expected_version.minor, req->version.minor);

    ConstStringResult cstr_res = http_request_HeaderGetValue(req, "host");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_EQUAL_STRING("example.com", cstr_res.Value);

    cstr_res = http_request_HeaderGetValue(req, "user-agent");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_EQUAL_STRING("MyTestClient/1.0", cstr_res.Value);

    cstr_res = http_request_HeaderGetValue(req, "content-length");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_EQUAL_STRING("28", cstr_res.Value);

    cstr_res = http_request_HeaderGetValue(req, "content-type");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_EQUAL_STRING("application/json; charset=utf-8", cstr_res.Value);

    cstr_res = http_request_HeaderGetValue(req, "x-custom-header");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_EQUAL_STRING("with leading space", cstr_res.Value);

    TEST_ASSERT_EQUAL_UINT(28, req->body.length);
    TEST_ASSERT_NOT_NULL(req->body.data);
}

void test_http_request_parse_WrongContentLength_Fail(void) {
    const char *exampleRequest =
        "POST /api/v1/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: MyTestClient/1.0\r\n"
        "Content-Length: 35\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-CUSTOM-HEADER:    with leading space\r\n"
        "\r\n"
        "{\"name\": \"Alice\", \"age\": 30}";

    TEST_ASSERT_NOT_NULL(
        (http_request_parse(req, exampleRequest, strlen(exampleRequest))));
}

void test_http_request_parse_EmptyHeader_Success(void) {
    const char *exampleRequest = "POST /api/v1/users HTTP/1.1\r\n\r\n";

    TEST_ASSERT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));

    http_version expected_version;
    expected_version.major = 1;
    expected_version.minor = 1;

    TEST_ASSERT_EQUAL_STRING("POST", req->method);
    TEST_ASSERT_EQUAL_STRING("/api/v1/users", req->uri);
    TEST_ASSERT_EQUAL_UINT8(expected_version.major, req->version.major);
    TEST_ASSERT_EQUAL_UINT8(expected_version.minor, req->version.minor);

    TEST_ASSERT_NULL(req->header);

    TEST_ASSERT_EQUAL_UINT(0, req->body.length);
    TEST_ASSERT_NULL(req->body.data);
}

void test_http_request_parse_IncorrectRequestLine_Fail(void) {
    const char *exampleRequest = "POST /api/v1/usersJUNKHTTP/1.1\r\n\r\n";

    TEST_ASSERT_NOT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));
}

void test_http_request_parse_MissingSpace_Fail(void) {
    const char *exampleRequest = "POST /api/v1/usersHTTP/1.1\r\n\r\n";

    TEST_ASSERT_NOT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));
}

void test_http_request_parse_EmptyHeaderKey_Fail(void) {
    const char *exampleRequest = "POST /api/v1/users HTTP/1.1\r\n"
                                 "Host: example.com\r\n"
                                 ": value\r\n"
                                 "\r\n";

    TEST_ASSERT_NOT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));
}

void test_http_request_parse_WhitespaceHeaderKey_Fail(void) {
    const char *exampleRequest = "POST /api/v1/users HTTP/1.1\r\n"
                                 "Host: example.com\r\n"
                                 " : value\r\n"
                                 "\r\n";

    TEST_ASSERT_NOT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));
}

void test_http_request_parse_EmptyHeaderValue_Success(void) {
    const char *exampleRequest = "POST /api/v1/users HTTP/1.1\r\n"
                                 "Host: example.com\r\n"
                                 "X-No-Value:\r\n"
                                 "\r\n";

    TEST_ASSERT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));

    ConstStringResult cstr_res = http_request_HeaderGetValue(req, "x-no-value");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_EQUAL_STRING("", cstr_res.Value);
}

void test_http_request_parse_Body_Success(void) {
    const char *exampleRequest =
        "POST /users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 49\r\n"
        "\r\n"
        "name=FirstName+LastName&email=bsmth%40example.com";

    TEST_ASSERT_NULL(
        http_request_parse(req, exampleRequest, strlen(exampleRequest)));

    TEST_ASSERT_NOT_NULL(req->body.data);
    TEST_ASSERT_EQUAL_MEMORY(
        "name=FirstName+LastName&email=bsmth%40example.com", req->body.data,
        req->body.length);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_http_request_parse_Success);
    RUN_TEST(test_http_request_parse_WrongContentLength_Fail);
    RUN_TEST(test_http_request_parse_EmptyHeader_Success);
    RUN_TEST(test_http_request_parse_IncorrectRequestLine_Fail);
    RUN_TEST(test_http_request_parse_MissingSpace_Fail);
    RUN_TEST(test_http_request_parse_EmptyHeaderKey_Fail);
    RUN_TEST(test_http_request_parse_WhitespaceHeaderKey_Fail);
    RUN_TEST(test_http_request_parse_EmptyHeaderValue_Success);
    RUN_TEST(test_http_request_parse_Body_Success);

    return UNITY_END();
}
