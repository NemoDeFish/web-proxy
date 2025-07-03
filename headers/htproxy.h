#ifndef HTPROXY_H
#define HTPROXY_H

#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include "linkedlist.h"
#include "cache.h"
#include "htproxyutils.h"

#define BUF_SIZE 8192
#define HEADER_SIZE 16384
#define MAX_REQUEST_SIZE 2000
#define RESPONSE_SIZE 102400
#define NUM_ENTRIES 10

// Declaration of cache_entry structure
typedef struct cache_entry cache_entry;

// Handle a client connection after accepting 
void serve_client(int listenfd, int cache_size, Node *recent_request_tail, int caching_enabled);

// Starts the proxy server and listens on a port. Calls 
void listen_for_requests(const char *port, int caching_enabled);

// Reads the request header from the client
size_t read_req_header(char *buffer, size_t buffer_size, int clientfd);

// Handles the logic for cache misses or stale cache entries, based on spec logic
void handle_cache_miss_or_stale(cache_entry **cache, cache_entry *entry, int *cache_size, ssize_t total_read, char *buffer, 
                                 char *host_request, char *URI_request, 
                                 int cached, int caching_enabled, int cacheable, 
                                 int clientfd, char *response, ssize_t response_length);

// Determines if the request should be served by the cache or should forward the request to the server for a new copy
void handle_client_request(int clientfd, cache_entry** cache, int* cache_size, Node** recent_request_head, Node** recent_request_tail, int caching_enabled);

// Processes a response from the host, sends it to the client
void process_response(int connectfd, int clientfd, ssize_t* total_read, ssize_t* total_written, char* response, char* buffer);

// Handles the client's request by either retrieving the response from the server or the cache
ssize_t connect_to_host(char *buffer, const char *port, char *hostname, int clientfd, char* response);

#endif