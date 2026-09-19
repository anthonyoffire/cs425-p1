#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "lab.h"
#include "args.h"

char body_buf[MAX_BODY_LEN];

int print_usage(void){
    return printf("Usage: myapp -f <from> -t <to> [-s subject] [-b body] [-p port] [-H helo-host] <server>\n");
}

static char *read_body(void){
    if (fgets(body_buf, sizeof(body_buf), stdin) == NULL){
        if(ferror(stdin)){
            fprintf(stderr, "Error reading message body from stdin.");
            return NULL;
        }
        // Empty stdin, but no error
        body_buf[0]='\0';
    }
    return body_buf;
}

int parse_args(int argc, char *argv[], Msg_Info *info){
    if (!info)
        return -1;
    // Usage print case for make leak    
    if (argc == 1) {
        print_usage();
        return 0;
    }
    optind = 1;
    int opt;
    while((opt = getopt(argc, argv, "f:t:s:b:p:H:")) != -1){
        switch (opt){
            case 'f': info->from = optarg; break;
            case 't': info->to = optarg; break;
            case 's': info->subject = optarg; break;
            case 'b': info->body = optarg; break;
            case 'p': info->port = optarg; break;
            case 'H': info->host = optarg; break;
            default:
                // Invalid arg
                print_usage();
                return 1;
        }
    }
    // Check argument fulfillment
    if(optind >= argc){
        fprintf(stderr, "Expected argument after options\n");
        return 1;
    }
    info->server = argv[optind];
    // Check requirements
    if (!info->from || !info->to || !info->server){
        fprintf(stderr, "Error: Missing required arguments\n");
        print_usage();
        return 1;
    }
    

    if (!info->body){
        info->body = read_body();
        if (!info->body){
            fprintf(stderr, "Error reading message body.\n");
            return 1;
        }
    }
    return 0;
}