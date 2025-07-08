/*
    This file contains the functionality for the cache.
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

#define NUM_ENTRIES 10

/*
Function that frees a specific cache entry.
*/
void free_cache_entry(cache_entry* entry) {
    if (entry == NULL) return;
    free(entry->request);
    free(entry->response);
    free(entry->host);
    free(entry->request_uri);
    free(entry);
}

/*
Function that determines if a given max-age directive is 0 or not.
Param: directive, a max-age directive
Returns: an int that indicates if max-age was 0 (1) or if it was not (0).
*/
int is_max_age_0(char *directive) {
    // first check if directive is max-age
    char *max_age = get_header_item(directive, "max-age=", "\0");
    if (max_age) {
        // convert found max age to int, then check if it is 0
        int age_allowed = atoi(max_age);
        if (age_allowed <= 0){
            free(max_age);
            return 1;
        }
    } 

    free(max_age);
    return 0;
}

/*
Function that checks if the given directive contains any information that would make it unable to be cached. 
Params: directive, a given directive to be searched.
Returns: an int that indicates if the request is allowed to be cached. 
*/
int no_cache_directive(char *directive) {
    if (!directive) {
        return 0;
    }

    int cacheable = 1;

    char *revalid = case_insensitive_strstr(directive, "=must-revalidate");
    char *proxy_revalid = case_insensitive_strstr(directive, "=proxy-revalidate");
    char *no_store = case_insensitive_strstr(directive, "=no-store");
    char *no_cache = case_insensitive_strstr(directive, "=no-cache");
    char *private = case_insensitive_strstr(directive, "=private");

    // first check max age
    if (is_max_age_0(directive)){
        cacheable = 0;
    }
    if (revalid || proxy_revalid || no_cache || no_store || private) {
        cacheable = 0;
    }
    return cacheable;
}

/*
Function that determines from a header, if the request is cacheable by investigating the Cache-Control line. 
Params: header, the header of the request
Returns: an int that indicates if the request is able to be cached or not. 1 if yes.
*/
int is_valid_cache(char *header) {
    int valid = 1;

    // first obtain the Cache-Control line
    char *cache_control = get_header_item(header, "Cache-Control", "\r\n");
    if (!cache_control) {
        // assume no cache control portion in header means it is okay to cache
        return valid;
    }

    // now parse through and find all the directives
    char *commas = ",";
    char *directive = strtok(cache_control, commas);

    while (directive != NULL) {
        if (no_cache_directive(directive) == 0) {
            valid = 0;
            break;
        }
        directive = strtok(NULL, commas);
    }

    free(cache_control);
    return valid;
}

/*
Function that serves the cache to the client. 
Params: clientfd (file descriptor for client), entry (cache entry)
Returns: void
*/
void serve_cache(int clientfd, cache_entry* entry) {
    printf("Serving %s %s from cache\n", entry->host, entry->request_uri);
    fflush(stdout);

    char *response = entry->response;
    ssize_t response_size = entry->response_size;

    // then reply with the response that you received last time.
    int n = write(clientfd, response, response_size);
    if (n < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
}

/*
Function that searches the cache for a request, and if found, serves the request. 
Params: the cache, the request string, the pointer to the cached response, reponse size, host from the request,
URI from the request, clientfd the fd to write to the client, and cache size
Returns: 11 if a cache hit and serve, i+1 if a stale entry is found at index i, 0 if not found 
*/
int search_cache(cache_entry **cache, const char *request, char **response, size_t *response_size, 
                char *host_request, char *URI_request, int clientfd, int *cache_size) {
    for (int i = 0; i < NUM_ENTRIES; i++) {
        if (cache[i] && strcmp(cache[i]->request, request) == 0) {
            time_t now = time(NULL);
            if (cache[i] && cache[i]->time && cache[i]->max_age && now - cache[i]->time > cache[i]->max_age) {
                printf("Stale entry for %s %s\n", host_request, URI_request);
                fflush(stdout);

                int modified = check_modified(request, cache[i], host_request, URI_request, clientfd);
                // 1 means 304, 0 means 200
                if (modified) {
                    serve_cache(clientfd, cache[i]);
                    printf("Entry for %s %s unmodified\n", host_request, URI_request);
                    fflush(stdout);
                    return 11;
                }
                return i+1;
            } else {
                serve_cache(clientfd, cache[i]);
                return 11;
            }
        }
    }
    return 0;
}

/*
Function that adds a request to the LRU list, or moves an existing entry to the head.
Params: cache, total_read the length of the request, buffer, recent_request_head and recent_request_tail pointers to LRU nodes
*/
void add_newreq_LRU(cache_entry **cache, size_t total_read, char *buffer, Node** recent_request_head, 
    Node** recent_request_tail) {
    // On every request, find the request that was referenced in the list and move it to the head.
    char* new_req = malloc(total_read + 1);
    memcpy(new_req, buffer, total_read);
    new_req[total_read] = '\0';
    
    if (find_move_head(&(*recent_request_head), &(*recent_request_tail), new_req) == -1) {
        // If the request is new, add request to most recently requested queue
        // Here, should we do some optimization to check that we don't store infinite list of requested pages?
        add_head(&(*recent_request_head), &(*recent_request_tail), new_req);
    } else {
        free(new_req);
    }
}

/*
Function that evicts the LRU cache entry.
Params: cache, recent_request_tail (pointer to LRU tail node), and cache_size a pointer to the size of the cache
*/
void evict_LRU_cache(cache_entry **cache, Node** recent_request_tail, int *cache_size) {
    // evict the least recently used (LRU) element of the cache if there is no empty slot.
    // The eviction should occur even if the request is not cached.
    Node* current = *recent_request_tail;
    int eindex = -1;
    while (current != NULL) {
        for (int i = 0; i < NUM_ENTRIES; i++) {
            // Extract request from cache entry
            if (cache[i] && strcmp(cache[i]->request, current->req) == 0 ) {
                eindex = i;
            }
        }
        if (eindex != -1) {
            printf("Evicting %s %s from cache\n", cache[eindex]->host, cache[eindex]->request_uri);
            fflush(stdout);

            free_cache_entry(cache[eindex]);
            cache[eindex] = NULL;
            (*cache_size)--;
            break;
        }
        current = current->prev;
    }
}

/*
Function that allocates memory for a new cache entry, and it copies the request into it 
Params: the cache, total_read the length of the request, buffer the request buffer
Returns: a pointer to a new cache_entry
*/
cache_entry *allocate_cache_entry(cache_entry **cache, size_t total_read, char *buffer) {
    cache_entry *entry = (cache_entry *)malloc(sizeof(cache_entry));
    entry->request = malloc(total_read + 1);        
    memcpy(entry->request, buffer, total_read);     
    entry->request[total_read] = '\0';

    return entry;
}

/*
Function that adds a new cache entry into the cache.
Params: the cache, total_read the length of the request, buffer the request buffer, the host and uri of the request, 
the response length, the cache size, the response itself, and the entry the cache_entry pointer
*/
void add_to_cache(cache_entry **cache, size_t total_read, char *buffer, char *host_request, char *URI_request,
    ssize_t response_length, int *cache_size, char *response, cache_entry *entry) {
    // and if the request is less than 2000 bytes and the response
    // is 100 kiB or less, place it in the cache.
    entry->response = (char *)malloc(response_length);
    entry->response_size = response_length;
    memcpy(entry->response, response, response_length);
    entry->host = host_request;
    entry->request_uri = URI_request;
    entry->time = time(NULL);
    entry->max_age = 0;
    char* max_age_str = get_header_item(response, "max-age=", "\r\n");
    if (max_age_str) {
        entry->max_age = atoi(max_age_str);
    }
    free(max_age_str);
            
    for (int i = 0; i < NUM_ENTRIES; i++) {
        if (cache[i] == NULL) {
            cache[i] = entry;
            (*cache_size)++;
            break;
        }
    }
}

/*
Function that serves a client's request using the cache. 
Params: the cache, the amount read of the client's request, the request, the LRU head and tail nodes, the host and URI
from the request, the fd of the client, the size of the cache, the pointer to where the response will be stored, the size of
that response, and an integer to be changed if the request is previously cached
Returns: 0 if a miss, 11 if a valid cache hit and served, or index of the cache hit + 1 (stale)
*/
int serve_cached_requests(cache_entry **cache, size_t total_read, char *buffer, Node** recent_request_head, 
    Node** recent_request_tail, char *host_request, char *URI_request, int clientfd, int *cache_size,
    char *cached_response, size_t *cached_response_size, int cached) {

    // log a new request if not in cache
    add_newreq_LRU(cache, total_read, buffer, recent_request_head, recent_request_tail);

    // check if request is small enough for cache
    if (total_read < MAX_REQUEST_SIZE) {
    // look in the cache to see if you have received this request before 
        // if you have, serve the request
        cached = search_cache(cache, buffer, &cached_response, cached_response_size,
                              host_request, URI_request, clientfd, cache_size);
        // request is not found in cache
        if (!cached) {
            // check if cache is full
            if (*cache_size >= NUM_ENTRIES) {
                evict_LRU_cache(cache, recent_request_tail, cache_size);
            }
        }
    }

    return cached;
}
