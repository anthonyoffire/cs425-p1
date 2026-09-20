#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "harness/unity.h"
#include "../src/args.h"

static void test_print_usage(void){
    TEST_ASSERT_EQUAL_INT16(0, print_usage());
}
static void test_read_body(void) {
    char *res;
    FILE *f_valid;
    FILE *f_empty;

    f_valid = fmemopen("Hello World\n", 12, "r");
    res = read_body(f_valid);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING("Hello World\n", res);
    fclose(f_valid);

    f_empty = fmemopen("", 0, "r");
    res = read_body(f_empty);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING("", res);
    fclose(f_empty);
}
static void test_parse_args(void) {
    Msg_Info info;

    char *argv_no_args[] = { "app", NULL };
    char *argv_unknown[] = { "app", "-z", "server", NULL };
    char *argv_no_positional[] = { "app", "-f", "a@b.com", NULL };
    char *argv_missing_req[] = { "app", "-f", "a@b.com", "mail.server.com", NULL };
    char *argv_full[] = { "app", "-f", "a@b.com", "-t", "c@d.com", "-s", "Sub", "-b", "Body", "-p", "587", "-H", "myhost", "mail.server.com", NULL };
    char *argv_defaults[] = { "app", "-f", "a@b.com", "-t", "c@d.com", "-b", "Body", "mail.server.com", NULL };

    /* 1. NULL info guard branch */
    TEST_ASSERT_EQUAL_INT(-1, parse_args(1, NULL, NULL));

    /* 2. No arguments (argc == 1) branch */
    memset(&info, 0, sizeof(info));
    TEST_ASSERT_EQUAL_INT(0, parse_args(1, argv_no_args, &info));

    /* 3. Unknown flag branch */
    memset(&info, 0, sizeof(info));
    TEST_ASSERT_EQUAL_INT(1, parse_args(3, argv_unknown, &info));

    /* 4. Missing positional argument branch (optind >= argc) */
    memset(&info, 0, sizeof(info));
    TEST_ASSERT_EQUAL_INT(1, parse_args(3, argv_no_positional, &info));

    /* 5. Missing required fields branch */
    memset(&info, 0, sizeof(info));
    TEST_ASSERT_EQUAL_INT(1, parse_args(4, argv_missing_req, &info));

    /* 6. Success path with all explicit flags */
    memset(&info, 0, sizeof(info));
    TEST_ASSERT_EQUAL_INT(0, parse_args(14, argv_full, &info));

    /* 7. Defaults fallback branch */
    memset(&info, 0, sizeof(info));
    TEST_ASSERT_EQUAL_INT(0, parse_args(8, argv_defaults, &info));
}
void run_args_tests(void) {
    RUN_TEST(test_print_usage);
    RUN_TEST(test_read_body);
    RUN_TEST(test_parse_args);
}


