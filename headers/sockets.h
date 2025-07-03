#ifndef SOCKETS_H
#define SOCKETS_H

// Creates a listen socket for the proxy listening for the client
int listen_socket(const char *port);

// Creates a connection socket for the proxy to connect with the host
int connect_socket(const char *port, char *hostname);

#endif