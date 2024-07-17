#include <pty.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>

// --- LOCAL INCLUDES ---
#include "fabrial.h"

// maxium number of events epoll will handle
#define MAX_EVENTS          10

// size of the message header
#define HDR_SIZE            5

// size of buffer to read data
#define BUFF_SIZE           4096

// reads from the pty and write to the socket
int read_pty(int sock, int pty, char *buff, size_t count) {
    ssize_t n;

    n = read(pty, buff, count);
    if (n == -1) {
        BAIL("read: pty");
    }

    n = write(sock, buff, n);
    if (n== -1) {
        BAIL("write pty to sock");
    }

    return 0;
}

// reads a data mesage from the socket
int read_sock_data(int sock, int pty, uint8_t *header, char *buff) {
    int n, err;
    uint16_t len;

    len = header[1] | (header[2] >> 8) | (header[3] >> 16) | (header[4] >> 24);

    n = read(sock, buff, len);
    if (n == -1) {
        BAIL("read: sock");
    }

    err = write(pty, buff, n);
    if (err == -1) {
        BAIL("write: sock to pty");
    }
    
    return 0;
}

// resize the pty
//
// @param   pty     pseudo-termainl file descriptor
// @param   header  new pty window size, as specified in a resize tty message
//
// @return  0 on success, < 0 on error
int resize_pty(int pty, uint8_t *header) {
    struct winsize winsz = {
        .ws_row = header[1] | (header[2] >> 8),
        .ws_col = header[3] | (header[4] >> 8),
    };
    

    INFO("resizing pty, rows = %d, cols = %d\n", winsz.ws_row, winsz.ws_col);

    return ioctl(pty, TIOCSWINSZ, &winsz);
}

// Read data from the socket and write to the pty
int read_sock(int sock, int pty, char *buff) {
    uint8_t header[HDR_SIZE];
    size_t n;

    // message structures
    //
    // Data Message:
    //         |-------|-------|-------|-------|-------|-------|-------|-------|
    // byte #: |   0   |   1   |   2   |   3   |   4   |   5   |  ...  |   n   |
    //         |-------|-------|-------|-------|-------|-------|-------|-------|
    // field : | 0x01  |       payload length          |        payload        |
    //         |-------|-------|-------|-------|-------|-----------------------|
    //
    // Resize TTY:
    //         |-------|-------|-------|-------|-------|
    // byte #: |   0   |   1   |   2   |   3   |   4   |
    //         |-------|-------|-------|-------|-------|
    // field : | 0x02  |      rows     |   columns     |
    //         |-------|-------|-------|-------|-------|

    // read the type of message
    n = read(sock, header, ARRSIZE(header));
    if (n == -1) {
        BAIL("read: sock (header)");
    } else if (n < HDR_SIZE) {
        BAIL("failed to header enough bytes for header");
    }

    switch (header[0]) {
        case MSG_TYPE_ZERO:
            // do nothing...
            break;
        case MSG_TYPE_DATA:
            if (read_sock_data(sock, pty, header, buff) == -1) {
                BAIL("unable to read socket data");
            }
            break;
        case MSG_TYPE_RESIZE:
            if (resize_pty(pty, header) < 0) {
                WARN("unable to resize pty");
            }
            break;
        default:
            WARN("sock: unknown msg type: %d\n", header[0]);
    }

    return 0;
}

// Executes the child process
int exec_child(char *prog) {
    char *const argv[2] = {prog, NULL};
    char *const envp[2] = {"TERM=xterm-256color", NULL};

    return execve(prog, argv, envp);
}

// Monitors pty and socket traffic
//
// Forwards traffic between the pty and the connected vhost socket
int run_parent(int sock, int pty) {
    char buff[BUFF_SIZE] = { 0 };
    struct epoll_event sock_ev, pty_ev, events[MAX_EVENTS];
    int epollfd, nfds, n, err, ret = 0;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        BAIL("epoll_create1");
    }

    sock_ev.events = EPOLLIN;
    sock_ev.data.fd = sock;

    err = epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &sock_ev);
    if (err == -1) {
        BAIL("epoll_ctl: sock");
    }

    pty_ev.events = EPOLLIN;
    pty_ev.data.fd = pty;
    err = epoll_ctl(epollfd, EPOLL_CTL_ADD, pty, &pty_ev);
    if (err == -1) {
        BAIL("epoll_ctl: pty");
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            BAIL("epoll_wait");
        }

        for (n = 0; n < nfds; ++n) {
            int fd = events[n].data.fd;

            if (fd == sock) {
                err = read_sock(sock, pty, buff);
            } else if (fd == pty) {
                err = read_pty(sock, pty, buff, ARRSIZE(buff));
            }

            if (err < 0) {
                ret = -1;
                goto cleanup;
            }
        }
    }

cleanup:
    close(epollfd);

    return ret;
}

// Spawns a new pty and handles bi-directional traffic
int spawn_pty(int sock) {
    int err, mfd, pid;

    pid = forkpty(&mfd, NULL, NULL, NULL);
    switch (pid) {
    case -1:
        BAIL("forkpty");
    case 0:
        /* child */
        err = exec_child("/bin/ash");
        if (err == -1) {
            perror("execve");
            exit(-1);
        }

        exit(0);
    default:
        /* parent */
        INFO("spawned pty, pid = %d\n", pid);
        break;
    }

    err = run_parent(sock, mfd);
    if (err == -1) {
        BAIL("unable to run parent");
    }

    return 0;
}
