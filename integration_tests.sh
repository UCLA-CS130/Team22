#!/bin/bash

#test basic port listening
echo "checking simple port listen on 8080 in config..."
echo -e "server {\n\tlisten 8080;\n}" > config_temp;

./webserver config_temp &
sleep 1

if netstat -vatn | grep 0.0.0.8080 > /dev/null; then
    echo "--tcp connection established at port 8080"
else
    echo "--no tcp connection found at port 8080"
fi

expected_curl_response="GET / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.43.0\nAccept: */*"

if curl -s localhost:8080 | grep $expected_curl_response ; then
    echo "--curl succeeded"
else
    echo "--curl failed"
fi

rm config_temp
kill %1

#test invalid config
#invalid_config1="server{ "
