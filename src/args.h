#include "lab.h"
#ifndef ARGS_H
#define ARGS_H

/**
 * MAX_BODY_LEN: Maximum len read from stdin for message body
 */
#define MAX_BODY_LEN 1024
int print_usage(void);
int parse_args(int, char**, Msg_Info*);

#endif