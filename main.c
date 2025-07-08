/*
    This file contains all the helper functions for the proxy.
    Written by Si Yong Lim and Ella McKercher.
*/

#include "headers/sockets.h"
#include "headers/htproxy.h"
#include "headers/htproxyutils.h"
#include "headers/cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
The driver code. Takes arguments in the form of ./htproxy -p listen-port [-c] 
where the -c is optional. 
*/
int main(int argc, char *argv[]) {
    const char *port = argv[2];
    
    // Check for optional -c flag
    int caching_enabled = 0;
    if (argc == 4) {
        // -c flag indicates caching is enabled
        if (strcmp(argv[3], "-c") == 0) {
            caching_enabled = 1;
        } else {
            fprintf(stderr, "Error: Invalid option %s\n", argv[3]);
            return EXIT_FAILURE;
        }
    }

     listen_for_requests(port, caching_enabled);

    return 0;
}
