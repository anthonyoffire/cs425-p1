#ifndef LAB_H
#define LAB_H

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
int send_mail(Msg_Info*);

#endif // LAB_H
