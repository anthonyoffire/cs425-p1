#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"
/* ------------------------------------------------------------------------
  * Mocks for lab.c
  * ------------------------------------------------------------------------ */
typedef struct {
    const char **script; /* Array of server responses */
    size_t step;         /* Current script index */
    char write_history[BUFFER_SIZE]; /* Captured commands sent by client */
    size_t write_pos;
} MockScriptContext;

static ssize_t mock_script_read(void *handle, void *buf, size_t count) {
    MockScriptContext *ctx = (MockScriptContext *)handle;
    if (!ctx->script[ctx->step]) return 0;

    const char *resp = ctx->script[ctx->step++];
    size_t len = strlen(resp);
    if (count < len) len = count;
    memcpy(buf, resp, len);
    return (ssize_t)len;
}

static ssize_t mock_script_write(void *handle, const void *buf, size_t count) {
    MockScriptContext *ctx = (MockScriptContext *)handle;
    if (ctx->write_pos + count < sizeof(ctx->write_history)) {
        memcpy(ctx->write_history + ctx->write_pos, buf, count);
        ctx->write_pos += count;
        ctx->write_history[ctx->write_pos] = '\0';
    }
    return (ssize_t)count;
}

static ssize_t mock_read(void *handle, void *buf, size_t count) {
    (void)handle;
    const char *mock_response = "220 smtp.example.com ESMTP ready\r\n";
    size_t len = strlen(mock_response);
    if (count < len) len = count;
    memcpy(buf, mock_response, len);
    return (ssize_t)len;
}

static ssize_t mock_write(void *handle, const void *buf, size_t count) {
    (void)handle;
    (void)buf;
    return (ssize_t)count; 
}
static ssize_t mock_write_fail(void *handle, const void *buf, size_t count) {
    (void)handle;
    (void)buf;
    return (ssize_t)(count - 1); // Incorrect count
}
static ssize_t mock_read_eof(void *handle, void *buf, size_t count) {
    (void)handle;
    (void)buf;
    (void)count;
    return 0; /* EOF / disconnect */
}

static ssize_t mock_read_multi_chunk(void *handle, void *buf, size_t count) {
    MockScriptContext *ctx = (MockScriptContext *)handle;
    if (!ctx->script[ctx->step]) return 0;

    const char *resp = ctx->script[ctx->step++];
    size_t len = strlen(resp);
    if (count < len) len = count;
    memcpy(buf, resp, len);
    return (ssize_t)len;
}
/* ------------------------------------------------------------------------
  * Unit Tests for lab.c
  * ------------------------------------------------------------------------ */
static void test_get_trimmed_len(void) {
    TEST_ASSERT_EQUAL_UINT(0, get_trimmed_len(NULL));
    TEST_ASSERT_EQUAL_UINT(0, get_trimmed_len(""));
    TEST_ASSERT_EQUAL_UINT(4, get_trimmed_len("test"));
    TEST_ASSERT_EQUAL_UINT(4, get_trimmed_len("test\r\n"));
}

static void test_parse_reply_code(void) {
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code(NULL));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("25"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("2A0"));
    TEST_ASSERT_EQUAL_INT(220, parse_reply_code("220 Ready\r\n"));
}
static void test_build_body(void) {
    char buf[512];
    Msg_Info info_sub = {
        .from = "a@b.com",
        .to = "c@d.com",
        .subject = "Hi\r\n",
        .body = "Body\n"
    };
    Msg_Info info_nosub = {
        .from = "a@b.com",
        .to = "c@d.com",
        .subject = NULL,
        .body = "Body"
    };

    TEST_ASSERT_EQUAL_INT(2, build_body(NULL, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_INT(0, build_body(&info_sub, buf, sizeof(buf)));
}
static void test_send_message(void) {
    MockScriptContext ctx = { NULL, 0 };
    Transport transport_ok = { NULL, mock_script_write, &ctx };
    Transport transport_fail = { NULL, mock_write_fail, &ctx };

    TEST_ASSERT_EQUAL_INT(0, send_message(&transport_ok, "HELO\r\n"));
    TEST_ASSERT_EQUAL_INT(2, send_message(&transport_fail, "HELO\r\n"));
}
static void test_read_line(void) {
    char recv_buf[128];
    LineReader reader;
    MockScriptContext ctx;
    Transport transport;

    const char *script_normal[] = { "220 ready\r\n", NULL };
    const char *script_partial[] = { "220 partial", NULL };

    reader.pos = 0;
    reader.len = 0;
    transport.read = mock_read_eof;
    transport.write = NULL;
    transport.handle = NULL;
    TEST_ASSERT_EQUAL_INT(2, read_line(&transport, &reader, recv_buf, sizeof(recv_buf)));

    ctx.script = script_normal;
    ctx.step = 0;
    reader.pos = 0;
    reader.len = 0;
    transport.read = mock_read_multi_chunk;
    transport.handle = &ctx;
    TEST_ASSERT_EQUAL_INT(0, read_line(&transport, &reader, recv_buf, sizeof(recv_buf)));

    ctx.script = script_partial;
    ctx.step = 0;
    reader.pos = 0;
    reader.len = 0;
    TEST_ASSERT_EQUAL_INT(0, read_line(&transport, &reader, recv_buf, sizeof(recv_buf)));

}
static void test_check_reply_code(void) {
    char recv_buf[128];
    LineReader reader;
    MockScriptContext ctx;
    Transport transport;

    const char *script_multiline[] = { "250-Hello\r\n", "250 OK\r\n", NULL };
    const char *script_mismatch[] = { "550 Error\r\n", NULL };

    ctx.script = script_multiline;
    ctx.step = 0;
    reader.pos = 0;
    reader.len = 0;
    transport.read = mock_read_multi_chunk;
    transport.write = NULL;
    transport.handle = &ctx;

    TEST_ASSERT_EQUAL_INT(0, check_reply_code(&transport, &reader, recv_buf, sizeof(recv_buf), 250));

    // Error branch
    reader.pos = 0;
    reader.len = 0;
    transport.read = mock_read_eof;
    transport.handle = NULL;

    TEST_ASSERT_EQUAL_INT(2, check_reply_code(&transport, &reader, recv_buf, sizeof(recv_buf), 250));
}
static void test_build_command(void) {
    char buf[128];

    TEST_ASSERT_EQUAL_INT(2, build_command(buf, sizeof(buf), NULL, "arg"));
    TEST_ASSERT_EQUAL_INT(0, build_command(buf, sizeof(buf), "MAIL FROM:", "a@b.com"));
}

static void test_smtp_session_success(void) {
    const char *script[] = {
        "220 ready\r\n", "250 hello\r\n", "250 ok\r\n",
        "250 ok\r\n", "354 go\r\n", "250 ok\r\n", "221 bye\r\n", NULL
    };
    MockScriptContext ctx = { .script = script, .step = 0 };
    Transport transport = { .read = mock_script_read, .write = mock_script_write, .handle = &ctx };
    Msg_Info info = { .from = "a@b.com", .to = "c@d.com", .body = "Hi" };

    TEST_ASSERT_EQUAL_INT(0, run_smtp_session(&transport, &info));
}

static void test_smtp_session_error(void) {
    const char *script[] = { "220 ready\r\n", "500 error\r\n", NULL };
    MockScriptContext ctx = { .script = script, .step = 0 };
    Transport transport = { .read = mock_script_read, .write = mock_script_write, .handle = &ctx };
    Msg_Info info = { .from = "a@b.com", .to = "c@d.com", .body = "Hi" };

    TEST_ASSERT_NOT_EQUAL(0, run_smtp_session(&transport, &info));
}

/* --- Suite Runner --- */

void run_lab_tests(void) {
    RUN_TEST(test_get_trimmed_len);
    RUN_TEST(test_parse_reply_code);
    RUN_TEST(test_build_body);
    RUN_TEST(test_send_message);
    RUN_TEST(test_read_line);
    RUN_TEST(test_check_reply_code);
    RUN_TEST(test_build_command);
    RUN_TEST(test_smtp_session_success);
    RUN_TEST(test_smtp_session_error);
}