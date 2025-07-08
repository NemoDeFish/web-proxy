/*
    This file contains all the helper functions for the proxy.
    Written by Si Yong Lim and Ella McKercher.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "headers/cache.h"
#include "headers/htproxyutils.h"

/*
Function that finds the start of the last line in a HTTP header.
Params: buffer (pointer to the start of the header), end (a pointer to the end of the header)
Returns: a pointer to the start of the last line of the header.
*/
char *find_header_last_line(char *buffer, char *end) {
    char *ptr = buffer;
    char *last_line_start = NULL;

    // move through the header to find the last "\r\n" signalling the last line
    while (ptr < end) {
        char *newline_indicator = strstr(ptr, "\r\n");

        // if there is no match, break 
        // or do not go past the end of the header (so excludes the "\r\n\r\n" at the end)
        // (this will maintain the previous \r\n so to truly find the last one
        if (!newline_indicator || newline_indicator >= end) {
            break;
        }

        // move past the \r\n (2 characters)
        last_line_start = newline_indicator + 2;
        // now move ptr to be the next line and try the loop again
        ptr = last_line_start;
    }

    return last_line_start;
}

/*
Function that logs the last line of the header (to match project 2's specs). 
Param: buffer, the header
*/
void get_header_tail(char *buffer) {
    char *end_of_header = strstr(buffer, "\r\n\r\n");
    // call find last line to get the start of the last line
    char *last_line_start = find_header_last_line(buffer, end_of_header);

    size_t len_last_line = end_of_header - last_line_start;
    char *last_line = malloc(len_last_line + 1);
    // if malloc didnt work
    if (!last_line) {
        perror("malloc for last line of header");
    }

    // copy over the last line of the header with null ptr as end
    strncpy(last_line, last_line_start, len_last_line);
    last_line[len_last_line] = '\0';

    printf("Request tail %s\n", last_line);
    
    fflush(stdout);

    free(last_line);
}

/*
Function that makes strstr() case insensitive. 
Params: str, the string to search, and substr to be searched for. 
Returns: a pointer to the first match in string str
*/
char* case_insensitive_strstr(char* str, char* substr) {
    while (*str) {
    if (tolower((unsigned char)*str) == tolower((unsigned char)*substr)) {
      const char *p1 = str + 1;
      const char *p2 = substr + 1;
      while (*p2 && tolower((unsigned char)*p1) == tolower((unsigned char)*p2)) {
        p1++;
        p2++;
      }
      if (*p2 == '\0') {
        return (char *)str;
      }
    }
    str++;
  }
  return NULL;
}

/*
Function that gets a header item from a given HTTP header. 
Params: buffer, the header, the wanted string (start of the wanted item), and the end string (end of the wanted item)
Returns: a new allocated string containing the wanted header item.
*/
char *get_header_item(char *buffer, char *wanted_string, char *end_string) {
    // finding the start and end of the header item
    char *item_ptr = case_insensitive_strstr(buffer, wanted_string);
    if (!item_ptr) {
        return NULL;
    }
    item_ptr = item_ptr + strlen(wanted_string);

    char *end_of_item = strstr(item_ptr, end_string);

    // now making the item char* to print
    size_t item_len = end_of_item - item_ptr;   

    char *item = malloc(item_len + 1);
    // if malloc didnt work
    if (!item) {
        perror("malloc for uri from header");
    }
    strncpy(item, item_ptr, item_len);
    item[item_len] = '\0';

    return item;
}

/*
Function that strips square brackets from an input.
Params: input, the original string to be stripped, output, the result string stripped of brackets
*/
void strip_brackets(const char *input, char *output) {
    size_t len = strlen(input);
    if (len >= 2 && input[0] == '[' && input[len - 1] == ']') {
        // Copy everything between the brackets
        strncpy(output, input + 1, len - 2);
        output[len - 2] = '\0'; // Null-terminate the string
    } else {
        // If no brackets, just copy the original string
        strcpy(output, input);
    }
}

/*
Function that frees the host, URI, and a cache entry and entry->response.
Params: host, URI, entry, all items that are freed in the function.
*/
void cleanup_request(char *host_request, char *URI_request, cache_entry *entry) {
    free(host_request);
    free(URI_request);
    free(entry->request);
    free(entry);
}

/*
Function that evicts a specified entry from the cache and frees it.
Params: the cache, the index of the cache entry to evict, the size of the cache (pointer)
*/
void evict_cache_entry(cache_entry **cache, int index, int *cache_size) {
    free_cache_entry(cache[index]);
    cache[index] = NULL;
    (*cache_size)--;
}

/*
Function that cleans up the header malloc'd items, the host and the URI.
Params: the host and URI of the request to be freed. 
*/
void cleanup_header(char *host_request, char *URI_request) {
    free(host_request);
    free(URI_request);
}
