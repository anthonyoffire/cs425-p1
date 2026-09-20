#ifndef LAB_H
#define LAB_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#ifdef TEST
  #define STATIC_T
#else
  #define STATIC_T static
#endif

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

/** * @brief Type for transport read function pointers.
 * @param context The context for the read operation.
 * @param read_buf The buffer to read data into.
 * @param count The number of bytes to read.
 * @return The number of bytes read, or -1 on error.
 */
typedef ssize_t (*TransportRead)(void *context, void *read_buf, size_t count);

/** * @brief Type for transport write function pointers.
 * @param context The context for the write operation.
 * @param write_buf The buffer containing data to write.
 * @param count The number of bytes to write.
 * @return The number of bytes written, or -1 on error.
 */
typedef ssize_t (*TransportWrite)(void *context, const void *write_buf, size_t count);

// Struct for holding read/write functions and socket handle/context pointer
typedef struct {
    TransportRead read;
    TransportWrite write;
    void *handle;
} Transport;

/** * @brief Structure to hold line reading state.
 */
typedef struct {
    char reader_buf[BUFFER_SIZE];
    size_t len;
    size_t pos;
} LineReader;

/** * @brief Structure to hold email message information.
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

/** * @brief Sends an email message using the provided message information.
 *
 * This function establishes a connection to the specified SMTP server and sends an email message
 * based on the provided message information. It handles the SMTP session, including sending commands
 * and receiving responses from the server.
 *
 * @param info A pointer to a Msg_Info structure containing the email message details.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the email was sent successfully.
 *         - 2: Failure, a connection error occurred while trying to send the email.
 */
int send_mail(Msg_Info*);

#ifdef TEST
#include <netdb.h>
/** * @brief Returns the length of a string after trimming trailing whitespace.
 *
 * @param str The string to trim.
 * @return The length of the trimmed string.
 */
STATIC_T size_t get_trimmed_len(const char *str);
/** * @brief Parses the reply code from a server response line.
 *
 * @param line The server response line.
 * @return The parsed reply code, or -1 if parsing fails.
 */
STATIC_T int parse_reply_code(const char *line);
/** * @brief Builds a command string for sending to the SMTP server.
 *
 * @param send_buf The buffer to store the command.
 * @param max_len The maximum length of the buffer.
 * @param prefix The command prefix.
 * @param arg The argument for the command.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the command was built successfully.
 *         - 2: Failure, an error occurred while building the command.
 */
STATIC_T int build_command(char *send_buf, size_t max_len, const char *prefix, const char *arg);
/** * @brief Builds the body of an email message.
 *
 * @param info A pointer to a Msg_Info structure containing the email message details.
 * @param output_buf The buffer to store the message body.
 * @param max_len The maximum length of the buffer.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the message body was built successfully.
 *         - 2: Failure, an error occurred while building the message body.
 */
STATIC_T int build_body(const Msg_Info *info, char *output_buf, size_t max_len);
/** * @brief Sends a message through the transport layer.
 *
 * @param transport A pointer to a Transport structure containing read/write functions and socket handle.
 * @param send_buf The buffer containing the message to send.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the message was sent successfully.
 *         - 2: Failure, an error occurred while sending the message.
 */
STATIC_T int send_message(Transport *transport, const char *send_buf);
/** * @brief Reads a line from the transport layer.
 *
 * @param transport A pointer to a Transport structure containing read/write functions and socket handle.
 * @param reader A pointer to a LineReader structure containing the reading state.
 * @param recv_buf The buffer to store the received line.
 * @param max_len The maximum length of the buffer.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, a line was read successfully.
 *         - 2: Failure, an error occurred while reading the line.
 */
STATIC_T int read_line(Transport *transport, LineReader *reader, char *recv_buf, size_t max_len);
/** * @brief Checks the reply code from a server response.
 *
 * @param transport A pointer to a Transport structure containing read/write functions and socket handle.
 * @param reader A pointer to a LineReader structure containing the reading state.
 * @param recv_buf The buffer containing the server response.
 * @param max_len The maximum length of the buffer.
 * @param expected_code The expected reply code.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the reply code matches the expected value.
 *         - 2: Failure, an error occurred or the reply code does not match.
 */
STATIC_T int check_reply_code(Transport *transport, LineReader *reader, char *recv_buf, size_t max_len, int expected_code);
/** * @brief Runs an SMTP session with the server.
 *
 * @param transport A pointer to a Transport structure containing read/write functions and socket handle.
 * @param info A pointer to a Msg_Info structure containing the email message details.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the SMTP session completed successfully.
 *         - 2: Failure, an error occurred during the SMTP session.
 */
STATIC_T int run_smtp_session(Transport *transport, const Msg_Info *info);
/** * @brief Handles sending commands and receiving responses during the SMTP session.
 *
 * @param transport A pointer to a Transport structure containing read/write functions and socket handle.
 * @param info A pointer to a Msg_Info structure containing the email message details.
 * @param reader A pointer to a LineReader structure containing the reading state.
 * @param send_buf The buffer to store the command to send.
 * @param send_max_len The maximum length of the send buffer.
 * @param com_str The command string.
 * @param arg The argument for the command.
 * @param recv_buf The buffer to store the received response.
 * @param recv_max_len The maximum length of the receive buffer.
 * @param expected_code The expected reply code.
 * @param is_command A flag indicating whether the operation is a command or body.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the operation completed successfully.
 *         - 2: Failure, an error occurred.
 */
STATIC_T int handle_send(Transport *transport, const Msg_Info *info, 
                LineReader *reader, char *send_buf, 
                size_t send_max_len, char *com_str, 
                const char *arg, char *recv_buf, 
                size_t recv_max_len, int expected_code, int is_command);  
/** * @brief Reads data from the socket.
 *
 * @param handle The socket file descriptor.
 * @param buf The buffer to store the received data.
 * @param count The number of bytes to read.
 * @return The number of bytes read, or -1 on error.
 */
STATIC_T ssize_t socket_read(void *handle, void *buf, size_t count);
/** * @brief Writes data to the socket.
 *
 * @param handle The socket file descriptor.
 * @param buf The buffer containing the data to write.
 * @param count The number of bytes to write.
 * @return The number of bytes written, or -1 on error.
 */
STATIC_T ssize_t socket_write(void *handle, const void *buf, size_t count);
/** * @brief Resolves a hostname and port to a list of addresses.
 *
 * @param host The hostname to resolve.
 * @param port The port to resolve.
 * @param res A pointer to a list of resolved addresses.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the addresses were resolved successfully.
 *         - 2: Failure, an error occurred while resolving the addresses.
 */
STATIC_T int socket_resolve(const char *host, const char *port, struct addrinfo **res);
/** * @brief Connects to a resolved address.
 *
 * @param addr_list A pointer to a list of resolved addresses.
 * @return An integer indicating the result of the operation:
 *         - 0: Success, the connection was established successfully.
 *         - 2: Failure, an error occurred while connecting.
 */
STATIC_T int socket_connect(struct addrinfo *addr_list);
#endif

#endif // LAB_H
