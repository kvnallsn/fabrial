#ifndef _FABRIAL_H_
#define _FABRIAL_H_

#include <errno.h>
#include <stdio.h>

#define MSG_TYPE_ZERO   0x00
#define MSG_TYPE_DATA   0x01
#define MSG_TYPE_RESIZE 0x02

// macro to print an error message and return
#define BAIL(msg, ...)                                           \
    do {                                                    \
        fprintf(stderr, "ERR: " msg "\n", ##__VA_ARGS__);   \
        fflush(stderr);                                     \
        return -1;                                          \
    } while (0)        

// macro to print a warning to stderr
#define WARN(msg, ...)                                      \
    do {                                                    \
        fprintf(stderr, "WARN: " msg "\n", ##__VA_ARGS__);  \
        fflush(stderr);                                     \
    } while (0)        

// macro to print an info message to stdout
#define INFO(msg, ...)                                      \
    do {                                                    \
        fprintf(stdout, "INFO: " msg "\n", ##__VA_ARGS__);  \
        fflush(stdout);                                     \
    } while (0)        

#define ARRSIZE(arr)    (sizeof(arr) / sizeof(*arr))

/// Spawns a new pseudo-terminal connected to a socket
int spawn_pty(int sock);

#endif
