/*
    Written by Si Yong Lim and Ella McKercher.
*/

#ifndef CACHE_H
#define CACHE_H

#include "htproxy.h"
#include "linkedlist.h"
#include "htproxyutils.h"

#define CACHE_HIT 11
#define STALE_CACHE_HIT_MIN 1
#define STALE_CACHE_HIT_MAX 10
#define CACHE_MISS 0

// The structure used for each cache entry
typedef struct cache_entry {
    char* request;
    char* response;
    char* host;
    char* request_uri;
    ssize_t response_size;
    time_t time;
    uint32_t max_age;
} cache_entry;

// Frees memory for a single cache entry
void free_cache_entry(cache_entry* entry);

// Checks if a Cache-Control directive contains a max-age value of 0
int is_max_age_0(char *directive);

// Checks if there is a no-cache directive in the header of the request
int no_cache_directive(char *directive);

// Checks from the header if caching is allowed
int is_valid_cache(char *header);

// Searches the cache for a matching request, and serves it if it is a match
int search_cache(cache_entry **cache, const char *request, char **response, size_t *response_size, 
                char *host_request, char *URI_request, int clientfd, int *cache_size);

// Adds a new request to the LRU list
void add_newreq_LRU(cache_entry **cache, size_t total_read, char *buffer, Node** recent_request_head, 
    Node** recent_request_tail);

// Evicts the least recently used cache entry
void evict_LRU_cache(cache_entry **cache, Node** recent_request_tail, int *cache_size);

// Allocates a new cache entry from the request buffer
cache_entry *allocate_cache_entry(cache_entry **cache, size_t total_read, char *buffer);

// Adds a new response to the cache
void add_to_cache(cache_entry **cache, size_t total_read, char *buffer, char *host_request, char *URI_request,
    ssize_t response_length, int *cache_size, char *response, cache_entry *entry);

// Serves requests from the cache (if valid), updates the LRU list 
int serve_cached_requests(cache_entry **cache, size_t total_read, char *buffer, Node** recent_request_head, 
    Node** recent_request_tail, char *host_request, char *URI_request, int clientfd, int *cache_size,
    char *cached_response, size_t *cached_response_size, int cached);

#endif