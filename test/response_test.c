#include "http/response.h"
#include "http/results.h"
#include "response/response_codes.h"
#include "response/response_internal.h"
#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <unity_internals.h>

HTTPResponseResult resp_res;
http_response *resp = NULL;

void setUp(void) {
    resp_res = http_response_new();
    if (!resp_res.Ok)
        exit(EXIT_FAILURE);
    resp = resp_res.Value;
}

void tearDown(void) { http_response_delete(resp); }

void test_http_response_bytes_Success(void) {
    TEST_ASSERT(!http_response_SetStatusCode(resp, HTTP_STATUS_OK));
    TEST_ASSERT(!http_response_SetReasonPhrase(resp, "OK"));
    TEST_ASSERT(!http_response_SetVersion(resp, 1, 1));

    const char* body =
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "   <head>"
        "       <meta charset='utf-8'/>"
        "       <title>Hello, World</title>"
        "   </head>"
        "   <body>"
        "       <h1>Hello, World!</h1>"
        "   </body>"
        "</html>";

    TEST_ASSERT(!http_response_SetBody(resp, (void*) body, strlen(body)));

    char contentLength[10];
    sprintf(contentLength, "%zu", resp->body.length);

    TEST_ASSERT(!http_response_HeaderSetValue(resp, "Connection", "close"));
    TEST_ASSERT(!http_response_HeaderSetValue(resp, "Content-Length", contentLength));
    TEST_ASSERT(!http_response_HeaderSetValue(resp, "Content-Type", "text/html; charset=UTF-8"));
    TEST_ASSERT(!http_response_HeaderSetValue(resp, "Date", "Mon, 13 Oct 2025 13:21:23 GMT"));

    const char* expected_response =
        "HTTP/1.1 200 OK\r\n"
        "date: Mon, 13 Oct 2025 13:21:23 GMT\r\n"
        "content-type: text/html; charset=UTF-8\r\n"
        "content-length: 169\r\n"
        "connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "   <head>"
        "       <meta charset='utf-8'/>"
        "       <title>Hello, World</title>"
        "   </head>"
        "   <body>"
        "       <h1>Hello, World!</h1>"
        "   </body>"
        "</html>";

    StringResult bytes_res = http_response_bytes(resp);
    TEST_ASSERT(bytes_res.Ok);
    sds bytes = bytes_res.Value;

    TEST_ASSERT_EQUAL_MEMORY(expected_response, bytes, strlen(expected_response));

    sdsfree(bytes);
}

void test_http_response_bytes_NoHeader_Success(void) {
    http_response_SetStatusCode(resp, HTTP_STATUS_OK);
    http_response_SetReasonPhrase(resp, "OK");
    http_response_SetVersion(resp, 1, 1);

    const char* expected_response = "HTTP/1.1 200 OK\r\n\r\n";

    StringResult bytes_res = http_response_bytes(resp);
    TEST_ASSERT(bytes_res.Ok);
    sds bytes = bytes_res.Value;

    TEST_ASSERT_EQUAL_MEMORY(expected_response, bytes, strlen(expected_response));

    sdsfree(bytes);
}

void test_http_response_bytes_BodyNoHeader_Fail(void) {
    http_response_SetStatusCode(resp, HTTP_STATUS_OK);
    http_response_SetReasonPhrase(resp, "OK");
    http_response_SetVersion(resp, 1, 1);

    const char* body =
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "   <head>"
        "       <meta charset='utf-8'/>"
        "       <title>Hello, World</title>"
        "   </head>"
        "   <body>"
        "       <h1>Hello, World!</h1>"
        "   </body>"
        "</html>";
    http_response_SetBody(resp, (void*) body, strlen(body));
    char contentLength[10];
    sprintf(contentLength, "%zu", resp->body.length);

    const char* expected_response =
        "HTTP/1.1 200 OK\r\n"
        "date: Mon, 13 Oct 2025 13:21:23 GMT\r\n"
        "content-type: text/html; charset=UTF-8\r\n"
        "content-length: 169\r\n"
        "connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html lang='en'>"
        "   <head>"
        "       <meta charset='utf-8'/>"
        "       <title>Hello, World</title>"
        "   </head>"
        "   <body>"
        "       <h1>Hello, World!</h1>"
        "   </body>"
        "</html>";

    StringResult bytes_res = http_response_bytes(resp);
    TEST_ASSERT(!bytes_res.Ok);
}

void test_http_response_bytes_CRLFHeader_Fail(void) {
    http_response_SetStatusCode(resp, HTTP_STATUS_OK);
    http_response_SetReasonPhrase(resp, "OK");
    http_response_SetVersion(resp, 1, 1);

    const char* expected_response = "HTTP/1.1 200 OK\r\n\r\n";
    http_response_HeaderSetValue(resp, "malicious\r\n", "value");

    ConstStringResult cstr_res = http_response_HeaderGetValue(resp, "malicious\r\n");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_NULL(cstr_res.Value);

    cstr_res = http_response_HeaderGetValue(resp, "malicious");
    TEST_ASSERT(cstr_res.Ok);
    TEST_ASSERT_NULL(cstr_res.Value);

    StringResult str_res = http_response_bytes(resp);
    TEST_ASSERT(str_res.Ok);
    sds bytes = str_res.Value;
    TEST_ASSERT_EQUAL_MEMORY(expected_response, bytes, strlen(expected_response));

    sdsfree(bytes);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_http_response_bytes_Success);
    RUN_TEST(test_http_response_bytes_NoHeader_Success);
    RUN_TEST(test_http_response_bytes_BodyNoHeader_Fail);
    RUN_TEST(test_http_response_bytes_CRLFHeader_Fail);

    return UNITY_END();
}
