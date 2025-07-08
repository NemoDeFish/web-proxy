# Written by Si Yong Lim.
#!/bin/bash

SECTION="$1"

run_task1() {
    valgrind --leak-check=full ./htproxy -p 8080 &
    PROXY_PID=$!
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task2_binary2() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s -X POST http://localhost/seed -H 'Content-Type: text/plain' --data '30023'
    curl -s --proxy http://localhost:8080 http://localhost/gen/100000 > response0.txt
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/gen/10000 > response1.txt
    sleep 1
    curl -s -X POST http://localhost/seed -H 'Content-Type: text/plain' --data '42'
    curl -s --proxy http://localhost:8080 http://localhost/gen/100000 > response2.txt
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/gen/10000 > response3.txt
    sleep 1
    echo "Verifying responses:"
    echo "Response 0 hash: $(sha256sum response0.txt | awk '{print $1}')"
    echo "Expected:        54d86c31f221503373aa265aa2956c3cbf676b8b8827da605597577d3cde77c9"
    echo
    echo "Response 1 hash: $(sha256sum response1.txt | awk '{print $1}')"
    echo "Expected:        91c0df593918e55bc230b68b65c1d6a0b52a4ffaa2bc6a4d7869c3b5018c2885"
    echo
    echo "Response 2 hash: $(sha256sum response2.txt | awk '{print $1}')"
    echo "Expected:        54d86c31f221503373aa265aa2956c3cbf676b8b8827da605597577d3cde77c9"
    echo
    echo "Response 3 hash: $(sha256sum response3.txt | awk '{print $1}')"
    echo "Expected:        91c0df593918e55bc230b68b65c1d6a0b52a4ffaa2bc6a4d7869c3b5018c2885"
    echo
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task2_evict_cache() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1  # give it time to start
    curl -s --proxy http://localhost:8080 http://localhost/echo/0
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/1
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/2
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/3
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/4
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/5
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/6
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/7
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/8
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/9
    sleep 1
    curl -X POST http://localhost/a --data '1'
    curl --proxy http://localhost:8080 http://localhost/a
    sleep 2
    curl -X POST http://localhost/a --data '2'
    curl --proxy http://localhost:8080 http://localhost/a
    sleep 1
    curl --proxy http://localhost:8080 http://localhost/echo/0
    sleep 1

    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task2_evict_nocache() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1  # give it time to start
    curl -s --proxy http://localhost:8080 http://localhost/echo/0
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/1
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/2
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/3
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/4
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/5
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/6
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/7
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/8
    sleep 1
    curl -s --proxy http://localhost:8080 http://localhost/echo/9
    sleep 1
    curl -X POST http://localhost/a --data '120000'
    curl --proxy http://localhost:8080 http://localhost/a
    sleep 2
    curl -X POST http://localhost/a --data '1'
    curl --proxy http://localhost:8080 http://localhost/a
    sleep 1
    curl --proxy http://localhost:8080 http://localhost/echo/0
    sleep 1

    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task2_res_long() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -X POST http://localhost/seed -H 'Content-Type: text/plain' --data '30023'
    curl -s --proxy http://localhost:8080 http://localhost/gen-plain/102399 > response0.txt
    sleep 1
    curl -X POST http://localhost/seed -H 'Content-Type: text/plain' --data '30024'
    curl -s --proxy http://localhost:8080 http://localhost/gen-plain/102399 > response1.txt
    echo "Verifying responses:"
    echo "Response 0 hash: $(sha256sum response0.txt | awk '{print $1}')"
    echo "Expected:        b8599f8f1ae664c437a3d29b61b215901bdb581f4b9b36e3f77a38088bac3b07"
    echo
    echo "Response 1 hash: $(sha256sum response1.txt | awk '{print $1}')"
    echo "Expected:        bc26d5579c566cc70bca75d7c6614264def44dcdadc10eb05fc28dce09c275a7"
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task2_req2_cache() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -X POST http://localhost/flip --data '0'
    curl --proxy http://localhost:8080 http://localhost/flip
    sleep 1
    curl -X POST http://localhost/flip --data '1'
    curl --proxy http://localhost:8080 http://localhost/flip
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task3() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s --proxy http://localhost:8080 "http://localhost/echo/Cache-Control%3A%20private"
    sleep 1
    curl -s --proxy http://localhost:8080 "http://localhost/echo/Cache-Control%3A%20private"
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task4_expire1() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s -X POST http://localhost/echo4 --data '(0)'
    curl -s -X POST http://localhost/echo4 --data 'max-age=4'
    curl -s --proxy http://localhost:8080 http://localhost/echo4/1 --header 'X-COMP30023: Hi'
    sleep 1
    curl -s -X POST http://localhost/echo4 --data '(1)'
    curl -s -X POST http://localhost/echo4 --data 'max-age=1'
    curl -s --proxy http://localhost:8080 http://localhost/echo4/2 --header 'X-COMP30023: Hi'
    sleep 2
    curl -s -X POST http://localhost/echo4 --data '(2)'
    curl -s -X POST http://localhost/echo4 --data 'max-age=1'
    curl -s --proxy http://localhost:8080 http://localhost/echo4/1 --header 'X-COMP30023: Hi'
    sleep 1
    curl -s -X POST http://localhost/echo4 --data '(2)'
    curl -s -X POST http://localhost/echo4 --data 'max-age=1'
    curl -s --proxy http://localhost:8080 http://localhost/echo4/2 --header 'X-COMP30023: Hi'
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task4_stale_complex() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s -X POST http://localhost/echo4 --data '(0)'
    curl -s -X POST http://localhost/echo4 --data 'MAX-AGE=1'
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023: HI'
    sleep 2
    curl -s -X POST http://localhost/echo4 --data ''
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023: HI'
    sleep 2
    curl -s -X POST http://localhost/echo4 --data '(1)'
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023: HI'
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task4_stale_evict1() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s -X POST http://localhost/echo4 --data '(0)'
    curl -s -X POST http://localhost/echo4 --data 'max-age=1'
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023-1: ?'
    sleep 2

    # Stale, not cached due to response size
    curl -s -X POST http://localhost/echo4 --data '(102399)'
    curl -s -X POST http://localhost/echo4 --data ''
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023-1: ?' > response0.txt
    sleep 1
    echo "Verifying responses:"
    echo "Response 0 hash: $(sha256sum response0.txt | awk '{print $1}')"
    echo "Expected:        96c0d57111501dd9620b714218eedb1f11037a6a1cee5d2abe8036a71d27ac82"

    # Should not serve from cache
    curl -s -X POST http://localhost/echo4 --data '(0)'
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023-1: ?'
    sleep 1

    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task4_stale() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s -X POST http://localhost/echo4 --data '(0)'
    curl -s -X POST http://localhost/echo4 --data 'max-age=1'
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023: Hi'
    sleep 2
    curl -s -X POST http://localhost/echo4 --data ''
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023: Hi'
    curl -s -X POST http://localhost/echo4 --data '(1)'
    curl -s --proxy http://localhost:8080 http://localhost/echo4 --header 'X-COMP30023: Hi'
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

run_task8() {
    valgrind --leak-check=full ./htproxy -p 8080 -c &
    PROXY_PID=$!
    sleep 1
    curl -s --proxy http://localhost:8080 http://example.com
    sleep 3
    curl -s --proxy http://localhost:8080 http://example.com
    sleep 10
    curl -s --proxy http://localhost:8080 http://example.com
    sleep 1
    kill -SIGINT $PROXY_PID
    wait $PROXY_PID 2>/dev/null
    echo "Proxy stopped."
}

case "$SECTION" in
    task1)
        run_task1
        ;;
    task2_binary2)
        run_task2_binary2
        ;;
    task2_evict_cache)
        run_task2_evict_cache
        ;;
    task2_evict_nocache)
        run_task2_evict_nocache
        ;;
    task2_req2_cache)
        run_task2_req2_cache
        ;;
    task2_res_long)
        run_task2_res_long
        ;;
    task3)
        run_task3
        ;;
    task4_expire1)
        run_task4_expire1
        ;;
    task4_stale_complex)
        run_task4_stale_complex
        ;;
    task4_stale_evict1)
        run_task4_stale_evict1
        ;;
    task4_stale)
        run_task4_stale
        ;;
    task4)
        run_task4_expire1
        run_task4_stale_complex
        run_task4_stale_evict1
        run_task4_stale
        ;;
    task8)
        run_task8
esac