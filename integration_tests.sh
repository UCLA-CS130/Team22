#!/bin/bash

#test basic port listening
printf "\ntesting simple port listen on 8080 in config...\n"
printf "server {\n\tlisten 8080;\n}" > config_temp;

./webserver config_temp &
sleep 1

if netstat -vatn | grep 0.0.0.8080 > /dev/null; then
    printf "  --tcp connection established at port 8080\n"
else
    printf "  !!no tcp connection found at port 8080\n"
fi

expected_curl_response="GET / HTTP/1.1  Host: localhost:8080  User-Agent: curl/7.43.0  Accept:"

if curl -s localhost:8080 | tr "\n\r" " " | grep "$expected_curl_response" > /dev/null; then
    printf "  --curl succeeded\n"
else
    printf "  !!curl failed\n"
fi

rm config_temp
kill %1
wait $! 2>/dev/null

#testing invalid port number
printf "\ntesting invalid port: port 100000...\n"
printf "server {\n\tlisten 100000;\n}" > config_temp

./webserver config_temp > temp_output 2>&1 &
sleep 1

if cat temp_output | grep "invalid" > /dev/null; then
    printf "  --invalid port caught\n"
else
    printf "  !!invalid port not caught\n"
fi

rm config_temp
rm temp_output
kill %1
wait $! 2>/dev/null

#test invalid config syntax
printf "\ntesting invalid config syntax: mismatched braces...\n"

printf "server{ listen 8080;" > config_temp

./webserver config_temp > temp_output 2>&1 &
sleep 1

if cat temp_output | grep "Mismatched brackets" > /dev/null; then
    printf "  --brace mismatch caught\n"
else
    printf "  !!brace mismatch not caught\n"
fi

rm config_temp
rm temp_output
kill %1
wait $! 2>/dev/null
