#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include "../src/args.h"

void test_send_mail(void);
void test_print_usage(void);
void test_parse_args(void);

void setUp(void) {
  printf("Setting up tests...\n");
}

void tearDown(void) {
  printf("Tearing down tests...\n");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_send_mail);
  RUN_TEST(test_print_usage);
  RUN_TEST(test_parse_args);
  return UNITY_END();
}