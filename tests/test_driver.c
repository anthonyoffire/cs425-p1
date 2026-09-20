#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include "../src/args.h"

void run_lab_tests(void);
void run_args_tests(void);

//GCOVR_EXCL_START
void setUp(void) {
  printf("Setting up tests...\n");
}

void tearDown(void) {
  printf("Tearing down tests...\n");
}

int main(void) {
  UNITY_BEGIN();
  run_lab_tests();
  run_args_tests();
  return UNITY_END();
}
//GCOVR_EXCL_STOP