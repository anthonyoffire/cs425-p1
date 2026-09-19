#include "lab.h"
#include <stdio.h>
#include <string.h>
#include "args.h"

#ifdef TEST
#define main main_exclude
#endif



int main(int argc, char **argv){
    // Allocate info struct
    Msg_Info info;
    memset(&info, 0, sizeof(Msg_Info));

    int arg_stat = parse_args(argc, argv, &info);
    // Check for arg error
    if (arg_stat !=0 || argc == 1){
        return arg_stat;
    }
    printf("To: %s, From: %s, Server: %s", info.to, info.from, info.server);
    int mail_stat = send_mail(&info);
    // Check for mail error
    if (mail_stat != 0)
        return mail_stat;
    return 0;
}