#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "args.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>


int parse_reply_code(const char *line) {
    if (!line || strlen(line) < 3) 
        return -1;
    // Convert first 3 chars to int for easier comparison later
    int code = 0;
    for (int i = 0; i < 3; i++) {
        if (line[i] < '0' || line[i] > '9') 
            return -1; // NaN
        code = code * 10 + (line[i] - '0');
    }
    return code;
}

int is_final_reply_line(const char *line) {
    if (!line || strlen(line) < 4) return 0;
    return line[3] == ' ';
}

int build_command(char *send_buf, size_t max_len, const char *prefix, const char *arg){
    int bytes_written;
    if (!prefix){
        fprintf(stderr, "Error in build command - null prefix.");
        return 2;
    }
    if (arg) {
        bytes_written = snprintf(send_buf, max_len, "%s%s\r\n", prefix, arg);
    } else {
        bytes_written = snprintf(send_buf, max_len, "%s\r\n", prefix);
    }
    if (bytes_written <= 0 || (size_t)bytes_written >= max_len){
        fprintf(stderr, "Error building command: [%s]\n", prefix);
        return 2;
    }
    return 0;
}

int build_body(const Msg_Info *info, char *output_buf, size_t max_len) {
    if (!info || !output_buf || max_len == 0) 
        return 0;

    size_t body_len = strlen(info->body);
    while (body_len > 0 && (info->body[body_len - 1] == '\r' || info->body[body_len - 1] == '\n')) {
        body_len--;
    }
    int bytes_written = snprintf(
        output_buf, max_len,
        "%s%s"
        "From: %s\r\n"
        "To: %s\r\n\r\n"
        "%.*s\r\n" // Limit the body length to exclude any trailing \r or \n
        ".\r\n",
        info->subject ? "Subject: " : "",
        info->subject ? info->subject : "",
        info->from,
        info->to,
        (int)body_len, info->body
    );
    if (bytes_written <= 0 || (size_t)bytes_written >= max_len){
        fprintf(stderr, "Error building body. Bytes written: %d, max_len: %ld\n", bytes_written, max_len);
        return 2; 
    }
    return 0;
}
int send_message(Transport *transport, const char *send_buf){
    size_t len = strlen(send_buf);
    ssize_t bytes_sent = transport->write(transport->handle, send_buf, len);
    if (bytes_sent != (ssize_t)len){
        fprintf(stderr, "Error sending command: [%s]\n", send_buf);
        return 2;
    }
    return 0;
}
int read_line(Transport *transport, LineReader *reader, char *recv_buf, size_t max_len){
    size_t out_idx = 0;
    while (out_idx < max_len - 1){
        if (reader->pos >= reader->len){
            ssize_t bytes_read = transport->read(transport->handle, reader->reader_buf, sizeof(reader->reader_buf));
            if (bytes_read <= 0){
                if (out_idx > 0)
                    break;
                return 2;
            }
            reader->len = (size_t) bytes_read;
            reader->pos = 0;
        }
        char c = reader->reader_buf[reader->pos++];
        recv_buf[out_idx++] = c;
        if (c == '\n')
            break;
    }
    recv_buf[out_idx] = '\0';
    return (out_idx > 0) ? 0 : 2;
}
int check_reply_code(Transport *transport, LineReader *reader, char *recv_buf, size_t max_len, int expected_code){
    while(1){
        if (read_line(transport, reader, recv_buf, max_len) != 0){
            fprintf(stderr, "Error finding last line from recv buffer.\n");
            return 2;
        }
        if (is_final_reply_line(recv_buf))
            break;
    }
    int code = parse_reply_code(recv_buf);
    if (code != expected_code){
        fprintf(stderr, "Reply code was %d, expected %d\n", code, expected_code);
        return 2;
    }
    return 0;
}
int handle_send(Transport *transport, const Msg_Info *info, LineReader *reader, char *send_buf, size_t send_max_len, char *com_str, const char *arg, char *recv_buf, size_t recv_max_len, int expected_code, int is_command){
    int res;
    if (is_command){
        if((res = build_command(send_buf, send_max_len, com_str, arg)) != 0) return res;
    } else {
        if((res = build_body(info, send_buf, send_max_len)) != 0) return res;
    }
    if((res = send_message(transport, send_buf)) != 0) return res;
    if((res = check_reply_code(transport, reader, recv_buf, recv_max_len, expected_code)) != 0) return res;
    return res;
}
int run_smtp_session(Transport *transport, const Msg_Info *info){
    LineReader reader;
    reader.len = 0;
    reader.pos = 0;

    char recv_buf[BUFFER_SIZE];
    char command_buf[BUFFER_SIZE];
    char body_buf[BUFFER_SIZE + MAX_BODY_LEN];
    int res;

    // Greeting
    if((res = check_reply_code(transport, &reader, recv_buf, sizeof(recv_buf), 220)) != 0) return res;
    
    // HELO
    if((res = handle_send(transport, NULL, &reader, command_buf, sizeof(command_buf), "HELO %s\r\n", info->host, recv_buf, sizeof(command_buf), 250, 1)) != 0) return res;
    
    // From
    if((res = handle_send(transport, NULL, &reader, command_buf, sizeof(command_buf), "MAIL FROM:<%s>\r\n", info->from, recv_buf, sizeof(command_buf), 250, 1)) != 0) return res;

    // To
    if((res = handle_send(transport, NULL, &reader, command_buf, sizeof(command_buf), "RCPT TO:<%s>\r\n", info->to, recv_buf, sizeof(command_buf), 250, 1)) != 0) return res;

    // Data
    if((res = handle_send(transport, NULL, &reader, command_buf, sizeof(command_buf), "DATA\r\n", NULL, recv_buf, sizeof(command_buf), 354, 1)) != 0) return res;

    // Body
    if((res = handle_send(transport, info, &reader, body_buf, sizeof(body_buf), NULL, NULL, recv_buf, sizeof(recv_buf), 250, 0)) != 0) return res;
    
    // Quit
    if((res = handle_send(transport, NULL, &reader, command_buf, sizeof(command_buf), "QUIT\r\n", NULL, recv_buf, sizeof(command_buf), 221, 1)) != 0) return res;
    return 0;
}
static ssize_t socket_read(void *handle, void *buf, size_t count) {
    int fd = *(int *)handle;
    return recv(fd, buf, count, 0);
}

static ssize_t socket_write(void *handle, const void *buf, size_t count) {
    int fd = *(int *)handle;
    return send(fd, buf, count, 0);
}

static int socket_resolve(const char *host, const char *port, struct addrinfo **res) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    return getaddrinfo(host, port, &hints, res);
}

static int socket_connect(struct addrinfo *addr_list) {
    for (struct addrinfo *rp = addr_list; rp != NULL; rp = rp->ai_next) {
        int sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            return sfd; // Success 
        }
        close(sfd);
    }
    return -1;
}
int send_mail(Msg_Info *info){
    if (!info) 
        return 2;
    struct addrinfo *addr_list = NULL;

    int res = socket_resolve(info->server, info->port, &addr_list);
    if (res != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        return 2;
    }
    int sfd = socket_connect(addr_list);
    freeaddrinfo(addr_list);
    if (sfd == -1) {
        fprintf(stderr, "Could not connect to server at %s:%s\n", info->server, info->port);
        return 2;
    }
    printf("Socket bound to server at %s on port %s\n", info->server, info->port);
    
    Transport transport = { socket_read, socket_write, (void*)(intptr_t)sfd };
    int status = run_smtp_session(&transport, info);
    close(sfd);
    return status;
}

