#!/bin/bash

error_flag=0

#test basic port listening
printf "\ntesting simple port listen on 8080 in config...\n"
printf "
			port 8080;\n
			threads 16;\n
			path /echo EchoHandler {}\n
			path /test EchoHandler {}\n
			path /static StaticHandler {\n
				root static;\n
			}\n
			path /special StaticHandler {\n
				root static/more;\n
			}\n
			path /static/same StaticHandler {\n
				root static;\n
			}\n
			path /sleep SleepHandler {\n
				seconds 1;\n
			}\n
			path /proxy1 ReverseProxyHandler {\n
				proxy_pass http://www.ucla.edu;\n
			}\n
			path /proxy2 ReverseProxyHandler {\n
				proxy_pass http://ucla.edu;\n
			}\n
			path /proxy3 ReverseProxyHandler {\n
				proxy_pass http://www.ucla.edu/static;\n
			}\n
			path /proxy4 ReverseProxyHandler {\n
				proxy_pass http://google.com;\n
			}\n
			default NotFoundHandler {}\n" > config_temp;

./webserver config_temp -s &
sleep 1

if netstat -vatn | grep 0.0.0.8080 > /dev/null; then
	printf "  --tcp connection established at port 8080\n"
else
	error_flag=1
	printf "  !!no tcp connection found at port 8080\n"
fi

if curl -s localhost:8080/echo | tr "\n\r" " " | grep "GET /echo HTTP/1.1" | grep "Host" | grep "User-Agent" > /dev/null; then
	printf "  --curl succeeded\n"
else
	error_flag=1
	printf "  !!curl failed\n"
fi

#testing file handler
printf "\ntesting file request of static/more/city.jpg ...\n"
if diff <(curl -s localhost:8080/special/city.jpg) static/more/city.jpg; then
	printf "  --city.jpg curled successfully\n"
else
	error_flag=1
	printf "  !!city.jpg failed to curl\n"
fi

sleep 1

#testing reverse proxy handler base
printf "\ntesting reverse proxy handler on http://www.ucla.edu ...\n"
if diff <(curl -s localhost:8080/proxy1) <(curl -s http://www.ucla.edu); then
	printf "  --ReverseProxyHandler base case curls properly\n"
else
	error_flag=1
	printf "  !!ReverseProxyHandler failed to curl properly\n"
fi

sleep 1

#testing reverse proxy handler subpath
printf "\ntesting reverse proxy handler subpath on http://www.ucla.edu ...\n"
if diff <(curl -s localhost:8080/proxy1/asdf.txt) <(curl -s http://www.ucla.edu/asdf.txt); then
	printf "  --ReverseProxyHandler curl subpath properly\n"
else
	error_flag=1
	printf "  !!ReverseProxyHandler failed to curl subpath properly\n"
fi

sleep 1

#testing reverse proxy handler 302 redirect
printf "\ntesting reverse proxy handler 302 redirect on http://www.ucla.edu ...\n"
if diff <(curl -s localhost:8080/proxy2) <(curl -s http://www.ucla.edu/); then # we curl www.ucla.edu because ucla.edu redirects there
	printf "  --ReverseProxyHandler curl 302 redirect properly\n"
else
	error_flag=1
	printf "  !!ReverseProxyHandler failed to curl 302 redirect properly\n"
fi

sleep 1

#testing reverse proxy handler subpath in config
printf "\ntesting reverse proxy handler subpath in config on http://www.ucla.edu ...\n"
if diff <(curl -s localhost:8080/proxy3) <(curl -s http://www.ucla.edu/static); then
	printf "  --ReverseProxyHandler curl subpath in config properly\n"
else
	error_flag=1
	printf "  !!ReverseProxyHandler failed to subpath in config properly\n"
fi

sleep 1

#testing reverse proxy handler subpath in config and url
printf "\ntesting reverse proxy handler subpath in config and url on http://www.ucla.edu ...\n"
if diff <(curl -s localhost:8080/proxy3/asdf.txt) <(curl -s http://www.ucla.edu/static/asdf.txt); then
	printf "  --ReverseProxyHandler curl subpath in config and url properly\n"
else
	error_flag=1
	printf "  !!ReverseProxyHandler failed to subpath in config and url properly\n"
fi

sleep 1

#testing reverse proxy handler subpath in config and url
printf "\ntesting reverse proxy handler subpath in config and url on http://www.ucla.edu ...\n"
if diff <(curl -s localhost:8080/proxy3/asdf.txt) <(curl -s http://www.ucla.edu/static/asdf.txt); then
	printf "  --ReverseProxyHandler curl subpath in config and url properly\n"
else
	error_flag=1
	printf "  !!ReverseProxyHandler failed to subpath in config and url properly\n"
fi

sleep 1

#testing longest prefix match
printf "\ntesting longest prefix match static/same/kinkakuji.jpg with uri 'static' and 'static/same' handler...\n"
if diff <(curl -s localhost:8080/static/kinkakuji.jpg) static/kinkakuji.jpg; then
	if diff <(curl -s localhost:8080/static/same/kinkakuji.jpg) static/kinkakuji.jpg; then
		printf "  --kinkakuji.jpg works with both prefixes\n"
	else
		error_flag=1
		printf "  !!kinkakuji.jpg only matched static but not static/same\n"
	fi

else
	printf "  !!kinkakuji.jpg failed to curl\n"
fi

# testing if multithreaded
printf "\ntesting parallel capabilities of the server (running 16 sleep handlers)\n"
processes=()
tooslow=0
for i in {1..15} ; do
	curl localhost:8080/sleep &
	processes+=($!)
done
sleep 2

for i in "${processes[@]}"; do
	if ps -p $i > /dev/null
	then
		echo "  !!server is too slow, server isn't running in multithreaded"
		kill $i
		tooslow=1
	fi
done
if [ $tooslow -ne 1 ] ; then
	echo "  --server is running multithreaded\n"
fi

# cleanup
rm config_temp
kill %1
wait $! 2>/dev/null

#testing invalid port number
printf "\ntesting invalid port: port 100000...\n"
printf "port 100000;" > config_temp

./webserver config_temp -s > temp_output 2>&1 &
wait $! 2>/dev/null

if cat temp_output | grep "Error" > /dev/null; then
	printf "  --invalid port caught\n"
else
	error_flag=1
	printf "  !!invalid port not caught\n"
	kill %1
fi


rm config_temp
rm temp_output


#test invalid config syntax
printf "\ntesting invalid config syntax: mismatched braces...\n"

printf "port 8080;\n path / EchoHandler {" > config_temp

./webserver config_temp -s > temp_output 2>&1 &
sleep 1

if cat temp_output | grep "Error parsing" > /dev/null; then
	printf "  --brace mismatch caught\n"
else
	error_flag=1
	printf "  !!brace mismatch not caught\n"
	kill %1
fi

rm config_temp
rm temp_output
wait $! 2>/dev/null

printf "\n"

if [ $error_flag -eq 1 ]
then
	exit 1
else
	exit 0
fi
