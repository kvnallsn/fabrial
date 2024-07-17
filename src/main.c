#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

// networking-related includes
#include <arpa/inet.h>
#include <sys/socket.h>
// needs to be defined after sys/socket.h, otherwise compile time errors
#include <linux/vm_sockets.h>

// --- LOCAL INCLUDES ---
#include "fabrial.h"

// maximum number of connections in the vhost socket backlog
#define LISTEN_BACKLOG      10

// maxium number of events epoll will handle
#define MAX_EVENTS          10

// container for arguments passed to a vsock connection thread
struct spawn_args {
    // connected vsock socket
    int sock;
};

// entry point for a vsock connection thread
//
// @param arg       pointer to struct spawn_args, containing thread arguments
static void *thread_main(void *arg) {
    int sock;

    if (arg == NULL) {
        WARN("no arguments passed to spawn thread\n");
        return NULL;
    }

    struct spawn_args *args = (struct spawn_args*)arg;
    sock = args->sock;

    free(args);

    if (spawn_pty(sock) == -1) {
        if (errno != 0) {
            perror("caused by");
        }
    }

    return NULL;
}

// builds a signalfd descriptor. 
//
// masks/blocks the following signals on the calling thread:
// - SIGINT
//
// @return signalfd on success, -1 on error
int mask_signals() {
    int sfd;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    // block signals so they aren't handled according to their default dispositions
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        BAIL("sigprocmask");
    }

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1) {
        BAIL("signalfd");
    }

    return sfd; 
}

// reads a signal from the signalfd
//
// @param   sfd       signalfd file descriptor
//
// @return  The id/number of the signal that was read, or -1 on error
int handle_signals(int sfd) {
    ssize_t s;
    struct signalfd_siginfo fdsi;

    s = read(sfd, &fdsi, sizeof(fdsi));
    if (s != sizeof(fdsi)) {
        BAIL("read: sfd (bad size)");
    }

    return fdsi.ssi_signo;
}

// Runs the vhost server
//
// - Waits for connections/signals 
// -- Spawns threads to handle connections
// -- Actions signals (if necessary)
//
// @param   sock    vhost-vsock socket file descriptor
//
// @return  0 on success, -1 on error
int run_vhost_server(int sock) {
    struct epoll_event sock_ev, sfd_ev, events[MAX_EVENTS];
    int epollfd, sfd, nfds, n, err;

    sfd = mask_signals();
    if (sfd == -1) {
        BAIL("unable to mask signals");
    }

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

    sfd_ev.events = EPOLLIN;
    sfd_ev.data.fd = sfd;
    err = epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &sfd_ev);
    if (err == -1) {
        BAIL("epoll_ctl: sfd");
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            goto cleanup;
        }

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == sock) {
                err = accept(sock, NULL, NULL);
                if (err == -1) {
                    perror("unable to accept connection");
                    continue;
                }

                struct spawn_args *args = malloc(sizeof(struct spawn_args));
                args->sock = err;

                pthread_t tid;
                pthread_create(&tid, NULL, &thread_main, (void*)args);
            }

            if (events[n].data.fd == sfd) {
                if (handle_signals(sfd) == SIGINT) {
                    goto cleanup;
                }
            }
        }
    }

cleanup:
    INFO("caught signal/error shutting down");
    close(sfd);

    return 0;
}

// binds a vhost-vsock socket and configures it listen
//
// @param   cid     Context Id of vsock device
// @param   port    Port to bind/listen
//
// @return  socket file descriptor on success, -1 on error
int bind_vhost_socket(uint32_t cid, uint32_t port) {
    int err = -1;
    int sock = -1;

    struct sockaddr_vm saddr = {
        .svm_family = AF_VSOCK,
        .svm_port = port,
        .svm_cid = cid,
    };

    sock = socket(AF_VSOCK, SOCK_STREAM, 0);
    if (sock < 0) {
        BAIL("unable to create socket");
    }

    err = bind(sock, (const struct sockaddr*)&saddr, sizeof(saddr));
    if (err == -1) {
        BAIL("unable to bind vhost socket");
    }

    err = listen(sock, LISTEN_BACKLOG);
    if (err == -1) {
        BAIL("unable to set vhost socket to listen");
    }

    return sock;
}

int main(int argc, char *argv[]) {
    int err, sock = -1;

    err = sock = bind_vhost_socket(VMADDR_CID_ANY, 3715);
    if (err == -1) {
        goto cleanup;
    }

    err = run_vhost_server(sock);
    close(sock);

cleanup:
    if (err < 0 && errno != 0) {
        perror("caused by");
    }

    fflush(stdout);
    fflush(stderr);

    return 0;
}
