#!/bin/bash

error_flag=0

#test basic port listening
printf "\ntesting simple port listen on 8080 in config...\n"
printf "server {\n\tlisten 8080;\n}" > config_temp;

./webserver config_temp &
sleep 1

if netstat -vatn | grep 0.0.0.8080 > /dev/null; then
    printf "  --tcp connection established at port 8080\n"
else
    error_flag=1
    printf "  !!no tcp connection found at port 8080\n"
fi

if curl -s localhost:8080 | tr "\n\r" " " | grep "GET / HTTP/1.1" | grep "Host" | grep "User-Agent" > /dev/null; then
    printf "  --curl succeeded\n"
else
    error_flag=1
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
    error_flag=1
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
    error_flag=1
    printf "  !!brace mismatch not caught\n"
fi

rm config_temp
rm temp_output
kill %1
wait $! 2>/dev/null

printf "\n"

if [ $error_flag -eq 1 ]
then
    exit 1
else
    exit 0
fi
