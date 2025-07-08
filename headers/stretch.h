/*
    Written by Si Yong Lim and Ella McKercher.
*/

#ifndef STRETCH_H
#define STRETCH_H

#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include "linkedlist.h"
#include "cache.h"
#include "htproxyutils.h"

#define UNMODIFIED 304
#define OKAY 200
#define UNMOD_LOG_LENGTH 64

// Declaration of the cache entry structure
typedef struct cache_entry cache_entry;

// Checks if, after sending a modified header to check if the response has been modified since the request,
// if the server sends back a 304 status code to indicate it has not been modified.
int check_304(ssize_t mod_response_len, char* mod_response, char *hostname, char *URI_request);

// Handles the request with the cache if the stale entry has not been modified.
int check_modified(const char *request, cache_entry *entry, char *host_request, char *URI_request, int clientfd);

#endif