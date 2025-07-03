#ifndef HTPROXYUTILS_H
#define HTPROXYUTILS_H

#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include "cache.h"
#include "htproxy.h"

// Declaration of the cache entry structure
typedef struct cache_entry cache_entry;

// Finds the last line of the header and returns it
char *find_header_last_line(char *buffer, char *end);

// Logs the last line of the header (to match project 2's specs)
void get_header_tail(char *buffer);

// Performs a case insensitive version of strstr()
char* case_insensitive_strstr(char* str, char* substr);

// After providing the start and end of a string, obtains the wanted item from a request header
char *get_header_item(char *buffer, char *wanted_string, char *end_string);

// Strips square brackets from input
void strip_brackets(const char *input, char *output);

// Frees up a cache entry, the host and uri that has been allocated
void cleanup_request(char *host, char *uri, cache_entry *entry);

// Evicts a cache entry
void evict_cache_entry(cache_entry **cache, int index, int *cache_size);

// Frees the host and uri that has been allocated
void cleanup_header(char *host_request, char *URI_request);

#endif