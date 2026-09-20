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
    printf("To: %s, From: %s, Server: %s\n", info.to, info.from, info.server);
    printf("Subj: %s, Body: %s, Port: %s, Host: %s\n", info.subject, info.body, info.port, info.host);
    int mail_stat = send_mail(&info);
    return mail_stat;
}