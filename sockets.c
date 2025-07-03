/*
This file contains all the functions that create needed sockets.
*/

#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "headers/htproxyutils.h"

/*
Function that creates a listen socket for the proxy listening for the client
Params: const char *port, which indicates the port the socket should bind to
Return: an int indicating the socket created
*/
int listen_socket(const char *port){
    int sockfd = 0, s;
    struct addrinfo hints, *res, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // This is a listening socket

    s = getaddrinfo(NULL, port, &hints, &res);
    if (s != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
                sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sockfd == -1)
                        continue;

                int enable = 1;
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
                    perror("setsockopt");
                    exit(1);
                }

                if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0){
                    break;
                }

                close(sockfd);
        }
        if (rp == NULL) {
                fprintf(stderr, "client: failed to connect\n");
                exit(EXIT_FAILURE);
        }
        freeaddrinfo(res);

    if (rp == NULL) {
        fprintf(stderr, "could not connect\n");
        return -1;
    }

    // Set the specified 10 backlog
    if (listen(sockfd, 10) < 0) {
        perror("listen"); // Match error reporting similar to the practical
        close(sockfd);
        return -1;
    }

    return sockfd;

}

/*
Function that creates a connection socket for the proxy to connect with the host
Params: const char *port, which specifies which port the socket should bind to, and char *hostname, which specifies which host to connect to
Return: an int that indicates the created socket
*/
int connect_socket(const char *port, char *hostname){
    struct addrinfo hints, *res;
    int sockfd = 0, s;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // getaddrinfo() requires stripped hostname brackets
    char stripped_hostname[100];
    strip_brackets(hostname, stripped_hostname);

    s = getaddrinfo(stripped_hostname, port, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sockfd, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
    return sockfd;
}
