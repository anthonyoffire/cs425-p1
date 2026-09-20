#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "lab.h"
#include "args.h"

char body_buf[MAX_BODY_LEN];

int print_usage(void){
    int printed = printf("Usage: myapp -f <from> -t <to> [-s subject] [-b body] [-p port] [-H helo-host] <server>\n");
    if (printed > 0)
        return 0;
    // GCOVR_EXCL_START
    return -1;
    // GCOVR_EXCL_STOP
}


STATIC_T char *read_body(FILE *source){
    if (fgets(body_buf, sizeof(body_buf), source) == NULL){
        if(ferror(stdin)){// GCOVR_EXCL_START
            fprintf(stderr, "Error reading message body from stdin.");
            return NULL;
        }// GCOVR_EXCL_STOP
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
    // Defaults
    if (!info->port)
        info->port = "25";
    if (!info->subject)
        info->subject = "";
    if (!info->host)
        info->host = "localhost";
    if (!info->body){// GCOVR_EXCL_START
        info->body = read_body(stdin);
        if (!info->body){ 
            fprintf(stderr, "Error reading message body.\n");
            return 1;
        }// GCOVR_EXCL_STOP
    }
    return 0;
}