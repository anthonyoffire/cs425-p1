#ifndef LAB_H
#define LAB_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Read and write wrapper function templates
typedef ssize_t (*TransportRead)(void *context, void *read_buf, size_t count);
typedef ssize_t (*TransportWrite)(void *context, const void *write_buf, size_t count);
// Struct for holding read/write functions and socket handle pointer
// void pointer for testing
typedef struct {
    TransportRead read;
    TransportWrite write;
    void *handle;
} Transport;

typedef struct {
    char reader_buf[BUFFER_SIZE];
    size_t len;
    size_t pos;
} LineReader;

/** * @brief Returns a greeting message.
 *
 * This function returns a string that contains a greeting message.
 * The string is allocated with malloc and should be freed by the caller.
 * @param name The name to include in the greeting.
 * @return A greeting string.
 */
char* get_greeting(const char* restrict name);

/**
 * 
 */
typedef struct {
    char *from;
    char *to;
    char *subject;
    char *body;
    char *port;
    char *host;
    char *server;
} Msg_Info;

/**
 * 
 */
int handle_send(Transport *transport, const Msg_Info *info, LineReader *reader, char *send_buf, size_t send_max_len, char *com_str, const char *arg, char *recv_buf, size_t recv_max_len, int expected_code, int is_command);
int send_message(Transport *transport, const char *send_buf);
int build_command(char *send_buf, size_t max_len, const char *prefix, const char *arg);
int send_mail(Msg_Info*);
int run_smtp_session(Transport *transport, const Msg_Info *info);
int check_reply_code(Transport *transport, LineReader *reader, char *recv_buf, size_t max_len, int expected_code);
int read_line(Transport *transport, LineReader *reader, char *recv_buf, size_t max_len);
int is_final_reply_line(const char *line);
int parse_reply_code(const char *line);
#endif // LAB_H
