# A Simple Caching HTTP/1.1 Web Proxy

## Project Overview

This project implements a simple caching web proxy for HTTP/1.1 (without persistent connections).

A web proxy acts as an intermediary for HTTP requests from clients seeking resources from other servers. This project adds basic caching capabilities and enforces specific behaviors based on HTTP headers.

---

## Features

### Basic Proxy

* Accepts connections on a specified port
* Parses `Host` header to forward HTTP requests to the correct origin server
* Forwards only `GET` requests
* Supports both IPv4 and IPv6
* Handles responses of arbitrary size (with optional 100 KiB truncation)
* Logs key proxy events to `stdout`

### Naive Caching

* Caches up to 10 unique `GET` requests (< 2000 bytes)
* Cache entries can store up to 100 KiB
* Implements LRU (Least Recently Used) eviction policy
* Serves cached content if identical request is repeated

### Valid Caching

* Honors `Cache-Control` headers such as `no-cache`, `no-store`, `private`, etc.
* Prevents caching of responses with prohibited directives

### Expiration

* Implements `max-age` expiration using `Cache-Control`
* Automatically invalidates expired cache entries
* Fetches fresh responses if a cached copy is stale

### Conditional GET

* Supports `If-Modified-Since` using `Date:` header from origin server
* If response is `304 Not Modified`, cached copy is used
* Useful for reducing network usage and improving efficiency

---

## Command Line Usage

```bash
./htproxy -p <listen-port> [-c]
```

* `-p <port>`: Port number to listen on
* `-c`: Enable caching (optional)

Example:

```bash
./htproxy -p 8080 -c
```

---

## Build Instructions

This project uses a `Makefile` for compilation.

### Build

```bash
make
```

### Clean

```bash
make clean
```

### Run

```bash
./htproxy -p 8080 -c
```

---

## Repository Structure

```
.
├── .github/
│   └── workflows/         # GitHub Actions workflows
├── headers/               # Header files
├── cases/                 # Test case input and response files
├── .gitignore             # Git ignored files specification
├── .gitattributes.txt     # Git attributes configuration
├── Makefile               # Build instructions
├── cache.c                # Cache logic
├── clang-format.txt       # Code formatting rules
├── collab.txt             # Collaboration notes or log
├── htproxy.c              # HTTP proxy main implementation
├── htproxyutils.c         # Utility functions for proxy
├── linkedlist.c           # Linked list implementation
├── main.c                 # Entry point or main logic
├── run\_test.sh            # Script to run tests
├── sockets.c              # Networking socket functions
├── stretch.c              # Possibly extra credit or additional features
```

---

## Testing

### How to Read the Test Cases

1. Open the directory corresponding to the test case you're interested in.
2. Open `seq.txt`.  
   This file contains the sequence of steps involved in the test case.  
   The line numbers reported by the CI correspond to the line number in this sequence file.

### Explanation of Common Commands

| Command                      | Description                                                                 |
|-----------------------------|-----------------------------------------------------------------------------|
| `start <port>`              | Start htproxy on `<port>` in background                                    |
| `stop`                      | Stop htproxy by sending SIGINT                                             |
| `log <filename>`            | Verify stdout of htproxy against `<filename>`                              |
| `log_accept <filename>`     | Verify "Accept" lines against `<filename>`                                  |
| `log_header <filename>`     | Verify "Request tail" lines against `<filename>`                            |
| `log_length <filename>`     | Verify "Response body length" lines against `<filename>`                    |
| `curl <...args>`            | Run curl command with `<...args>` in background (make request to proxy)     |
| `wait <n>`                  | Wait for curl command `n` to finish                                         |
| `response <n> <filename>`   | Verify response `n` against `<filename>`                                    |
| `response <n> <hash>`       | Verify response `n` against `<hash>`, the SHA-256 hash of the response      |
| `curl-post <...args>`       | Make a POST request to server on port 80 with `<...args>`                   |
| `relay-stream`              | Not provided, see Ed #693                                                   |

### How to Test

Manually work through each command in the sequence file from top to bottom.  
For example, if the command is:

```
start 8080

```

Start `htproxy` on port `8080`.
You can add the following section to your `README.md` to document how to run the test cases using the provided Bash script:

---

## Running Automated Testing

To test the `htproxy` implementation, you can use the provided Bash script with different test sections.

### Prerequisites

Ensure the following are available on your system:

* `valgrind`
* `curl`
* `sha256sum`
* `htproxy` binary compiled in the current directory

### Running a Test

The test script takes a single argument indicating which test case to run. For example:

```bash
chmod +x test.sh  # Make sure the script is executable

./test.sh task1
```

### Available Test Cases

| Test Case Name        | Description                                              |
| --------------------- | -------------------------------------------------------- |
| `task1`               | Basic proxy request forwarding                           |
| `task2_binary2`       | Caching verification with hash matching                  |
| `task2_evict_cache`   | Cache eviction behavior with small entries               |
| `task2_evict_nocache` | Cache eviction with oversized entries                    |
| `task2_res_long`      | Long responses with caching                              |
| `task2_req2_cache`    | Request-dependent cache consistency                      |
| `task3`               | Cache-control headers: `private`                         |
| `task4_expire1`       | Cache expiration timing (`max-age`)                      |
| `task4_stale_complex` | Stale cache validation with missing server updates       |
| `task4_stale_evict1`  | Stale cache entries that were never cached (size limits) |
| `task4_stale`         | Basic stale handling after expiration                    |
| `task8`               | Tests involving external websites like example.com       |

Each test will:

* Launch `htproxy` in the background using `valgrind`
* Perform various HTTP requests
* Wait for the process to complete
* Output the test results and any SHA256 hash checks

### Example

```bash
./test.sh task2_binary2
```

This runs a cache test and prints the SHA256 hash of the response to verify against expected values.
