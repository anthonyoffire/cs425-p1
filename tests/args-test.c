#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "harness/unity.h"
#include "../src/args.h"

void test_print_usage(void){
    TEST_ASSERT_EQUAL_INT16(0, print_usage());
}
void test_parse_args(void){
    Msg_Info info;
    memset(&info, 0, sizeof(Msg_Info));
    char *argv[] = {"myapp", "-f", "me", "-t", "u", "-b", "body", "server"};
    int stat = parse_args(sizeof(argv)/sizeof(argv[0]), argv, &info);
    TEST_ASSERT_EQUAL_INT16(0, stat);
}


