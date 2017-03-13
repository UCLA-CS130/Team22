#!/bin/bash

while true ; do 
printf "starting docker\n" >> crashlog
date -u >> crashlog
docker run -t -p 8080:8080 webserver >> log 2>&1
done

