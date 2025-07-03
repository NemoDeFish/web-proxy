/*
This file contains the main functionality for the proxy.
*/

#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "headers/sockets.h"
#include "headers/cache.h"
#include "headers/htproxy.h"
#include "headers/linkedlist.h"
#include "headers/htproxyutils.h"

// Define the cache
cache_entry* cache[NUM_ENTRIES] = {0};
Node *recent_request_head = NULL;


/*
Function that handles the frees after a Signal Interrupt. 
Ensures the program runs without segmentation faults.
*/
void handle_sigint(int sig) {
    for (int i = 0; i < NUM_ENTRIES; i++) {
        free_cache_entry(cache[i]);
    }

    freeList(recent_request_head);

    exit(0);  // Ensure the program exits after cleanup
}


/*
Function that accepts client requests and initiates serving the request.
Params: listenfd (listening socket), cache_size (size of the cache), 
recent_request_tail (last node in the linked list that holds all the recent requests)
caching_enabled (int that indicates if caching is enabled)
*/
void serve_client(int listenfd, int cache_size, Node *recent_request_tail, int caching_enabled) {
    while(1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_size = sizeof client_addr;

        int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (clientfd < 0) {
            perror("accept client");
            continue;
        }

        printf("Accepted\n");
        fflush(stdout);

        handle_client_request(clientfd, cache, &cache_size, &recent_request_head, &recent_request_tail, caching_enabled);
        
        close(clientfd);
    }
}

/*
Function that creates the listening socket and initiates listening for client requests.
Params: port (which port to bind the listening socket to), and caching_enabled (int that indicates if caching is enabled)
*/
void listen_for_requests(const char *port, int caching_enabled) {
    int cache_size = 0;
    Node *recent_request_tail = NULL;

    // Register the signal handler
    signal(SIGINT, handle_sigint);

    // Create the listening socket
    int listenfd = listen_socket(port);
    if (listenfd < 0) {
        perror("listening socket");
        exit(EXIT_FAILURE);
    }

    serve_client(listenfd, cache_size, recent_request_tail, caching_enabled);
}

/*
Function to read the request header from the client.
Params: buffer (char array to store the request header), buffer_size (the size of the char array),
clientfd (file descriptor of client's connection)
*/
size_t read_req_header(char *buffer, size_t buffer_size, int clientfd) {
    ssize_t amount_read = 0;
    size_t total_read = 0;

    while (1) {
        if (total_read >= buffer_size - 1) {
            perror("request too large");
            close(clientfd);
            return -1;
        }

        amount_read = read(clientfd, buffer + total_read, buffer_size - total_read - 1);
        if (amount_read <= 0) {
            perror("read");
            close(clientfd);
            return -1;
        }

        total_read += amount_read;
        buffer[total_read] = '\0';

        // look for end of header
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            return total_read;
        }
    }
}

/*
Function to handle the a cache miss or a stale cache
Params: cache (cache), entry (entry to be stored in the cache), cache_size (current size of cache),
total_read (total request size read from client), buffer (char array to store the request), 
host_request (request from client), URI_request (URI request from client),
cached (int to determine whether request was cached), caching_enabled (int to determine whether caching enabled)
cacheable (int to determine whether entry is cacheable), clientfd (file descriptor of client's connection),
response (char array of host response), response_length (size of reseponse)
*/
void handle_cache_miss_or_stale(cache_entry **cache, cache_entry *entry, int *cache_size, ssize_t total_read, char *buffer, 
                                 char *host_request, char *URI_request, 
                                 int cached, int caching_enabled, int cacheable, 
                                 int clientfd, char *response, ssize_t response_length) {
    // check if response can be cached in the first place
    if (cacheable && caching_enabled && total_read < MAX_REQUEST_SIZE && response_length < RESPONSE_SIZE) {
        // if it can be cached, but if it is stale, remove the stale entry 
        if (cached >= STALE_CACHE_HIT_MIN && cached <= STALE_CACHE_HIT_MAX) {
            evict_cache_entry(cache, cached-1, cache_size);
        }
        // and then add the fresh new response in place
        // (or if it was not stale, just add it to the cache)
        add_to_cache(cache, total_read, buffer, host_request, URI_request,response_length, 
                     cache_size, response, entry);
    // if there was a cache entry but we cannot cache the new response          
    } else if (cached) {
        printf("Evicting %s %s from cache\n", host_request, URI_request);
        fflush(stdout);

        evict_cache_entry(cache, cached-1, cache_size);
        cleanup_request(host_request, URI_request, entry);
    } else {
        // if it was not a cache hit and we cant cache the new response
        if (caching_enabled && !cacheable) {
            printf("Not caching %s %s\n", host_request, URI_request);
            fflush(stdout);
        }
        cleanup_request(host_request, URI_request, entry);
    }   

    // handle non-caching tasks
    int n = write(clientfd, response, response_length);
    if (n < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
}

/*
Function to determine if the request should be served by the cache or should forward the request to the server for a new copy.
Params: clientfd (file descriptor of client's connection), cache (cache), cache_size (current size of cache),
recent_request_head (head node in the linked list that holds all the recent requests)
recent_request_tail (last node in the linked list that holds all the recent requests)
caching_enabled (int that indicates if caching is enabled)
*/
void handle_client_request(int clientfd, cache_entry** cache, int* cache_size, Node** recent_request_head, Node** recent_request_tail, int caching_enabled) {
    // make large buffer so will definitely fit headers
    char buffer[BUF_SIZE];
    char *cached_response = NULL;
    size_t cached_response_size = 0;
    
    size_t total_read = read_req_header(buffer, BUF_SIZE, clientfd);
    if (total_read < 0) {
        return;
    }

    // perform tail request print to stdout
    get_header_tail(buffer);
    
    // get host and requested uri from header
    char *host_request = get_header_item(buffer, "Host: ", "\r\n");
    char *URI_request = get_header_item(buffer, "GET ", " ");

    if (!host_request || !URI_request) {
        fprintf(stderr, "no host or uri\n");
        close(clientfd);
        return;
    }

    int cached = 0;
    int cacheable = is_valid_cache(buffer);

    if (cacheable && caching_enabled) {
        cached = serve_cached_requests(cache, total_read, buffer, recent_request_head, recent_request_tail, 
            host_request, URI_request, clientfd, cache_size, cached_response, &cached_response_size, cached);
    }

    // cache miss or stale cache hit
    if (cached < CACHE_HIT) {
        cache_entry *entry = allocate_cache_entry(cache, total_read, buffer);

        char response[RESPONSE_SIZE];
        printf("GETting %s %s\n", host_request, URI_request);
        fflush(stdout);

        ssize_t response_length = connect_to_host(buffer, "80", host_request, clientfd, response);
        
        handle_cache_miss_or_stale(cache, entry, cache_size, total_read, buffer, host_request, URI_request, 
                                   cached, caching_enabled, cacheable, clientfd, response, response_length);
    } else {
        cleanup_header(host_request, URI_request);
    }
}

/*
Function to process a response from the host by sending it to the client.
Params: connectfd (file descriptor of host's connection), clientfd (file descriptor of client's connection), 
total_read (total request size read from client), total_written (total size written to client), 
response (char array of host response), buffer (char array to store the request), 
*/
void process_response(int connectfd, int clientfd, ssize_t* total_read, ssize_t* total_written, char* response, char* buffer) {
    // read response from host server
    ssize_t n = read(connectfd, buffer, BUF_SIZE);
    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // copy first part of response from host server to respone buffer
    if (*total_read + n <= RESPONSE_SIZE) {
        memcpy(response + *total_read, buffer, n);
        *total_read += n;
    } else {
        size_t remaining_space = RESPONSE_SIZE - *total_read;
        memcpy(response + *total_read, buffer, remaining_space);
        *total_read = RESPONSE_SIZE;
    }

    *total_written += n;

    // write first response to client
    n = write(clientfd, buffer, n);
    if (n < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

/*
Function to handle the client's request by either retrieving the response from the server or the cache.
Params: buffer (buffer containing the request to send to host), port (port number to connect to host),
hostname (name of host's server),  clientfd (file descriptor of client's connection), 
response (char to store the response from host server),
*/
ssize_t connect_to_host(char *buffer, const char *port, char *hostname, int clientfd, char* response) {
    // connect to host server
    int connectfd = connect_socket(port, hostname);
    if (connectfd < 0) {
        perror("connect socket");
        exit(EXIT_FAILURE);
    }
    
    // write request to host server
    ssize_t n = write(connectfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    ssize_t total_read = 0, total_written = 0;
    process_response(connectfd, clientfd, &total_read, &total_written, response, buffer);

    // Get Content-Length
    char *content_length_str = get_header_item(buffer, "Content-Length: ", "\r\n");
    size_t content_length = content_length_str ? atoi(content_length_str) : 0;
    printf("Response body length %zu\n", content_length);
    free(content_length_str);
    fflush(stdout);

    // if there are more stuff from the host server, keep reading and send to the client
    while (total_written < content_length) {
        process_response(connectfd, clientfd, &total_read, &total_written, response, buffer);
    }

    if (total_read < RESPONSE_SIZE) {
        response[total_read] = '\0';
    }

    close(connectfd);

    return total_read;
}