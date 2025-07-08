/*
    Written by Si Yong Lim and Ella McKercher.
*/
#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "headers/cache.h"
#include "headers/sockets.h"
#include "headers/htproxy.h"
#include "headers/htproxyutils.h"
#include "headers/linkedlist.h"
#include "headers/stretch.h"

/*
Function that adds the If-Modified-Since: date line to the header, for fetching optimization.
Params: buffer, the current header, buffer_size, the size of the buffer
*/
void add_if_mod_header(char *buffer, cache_entry *entry) {
    char *date = get_header_item(entry->response, "Date", "\r\n");

    // now create the line for the header
    char mod_line[UNMOD_LOG_LENGTH];
    snprintf(mod_line, sizeof(mod_line), "\r\nIf-Modified-Since%s\r\n\r\n", date);
    free(date);  

    // find the end of the header
    char *end = strstr(buffer, "\r\n\r\n");

    // in order for the header to be formatted correctly, I overwrite the first \r\n
    // so that manually adding the \r\ns like i did in the snprintf works
    *end = '\0';

    size_t buffer_len = strlen(buffer);
    size_t mod_line_len = strlen(mod_line);

    // check if the new line can fit (should just safety measure)
    if (buffer_len + mod_line_len >= BUF_SIZE) {
        fprintf(stderr, "buffer too small to add modified line\n");
        return;
    }

    // add the if modified at the end of the buffer
    int available = BUF_SIZE - buffer_len - 1;
    strncat(buffer, mod_line, available);
}

/*
Function that checks if the stale request has been modified by checking the server's response to the modified request.
Params: the modified response length, the modified response, the host of the request, the URI of the request
Returns: an int that indicates if the server returned 304 status code (1) or if it returned something else (0)
*/
int check_304(ssize_t mod_response_len, char* mod_response, char *hostname, char *URI_request) {
    char first_line[256];

    int i = 0;
    while (mod_response[i] != '\0' && mod_response[i] != '\n' && i < sizeof(first_line) - 1) {
        first_line[i] = mod_response[i];
        i++;
    }
    first_line[i] = '\0';

    if (strstr(first_line, "304") != NULL) {
        return 1;
    }

    return 0;
}

/*
Function that checks if the stretch goal optimization can occur. If it can, it handles the request with the cache.
Params: the port, the buffer (header), the host of the request, the uri of the request, and the client socket. 
Returns: an int that indicates if it has (1) been modified, or if it hasn't (0).
*/
int check_modified(const char *request, cache_entry *entry, char *host_request, char *URI_request, int clientfd) {
    // FIRST duplicate the header so that when adding the if modified line, things arent overwritten or break
    char copied_req[BUF_SIZE];
    memcpy(copied_req, request, BUF_SIZE - 1);
    copied_req[BUF_SIZE - 1] = '\0';

    // now add the if-modified line to the 
    add_if_mod_header(copied_req, entry);

    // connect to host server
    int connectfd = connect_socket("80", host_request);
    if (connectfd < 0) {
        perror("connect socket");
        exit(EXIT_FAILURE);
    }
    
    // write request to host server
    ssize_t n = write(connectfd, copied_req, strlen(copied_req));
    if (n < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // read response from host server
    n = read(connectfd, copied_req, BUF_SIZE);
    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    return check_304(n, copied_req, host_request, URI_request);
}