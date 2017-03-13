CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror -static-libgcc -static-libstdc++ -pthread -Wl,-Bstatic
LDLIBS= -lboost_log_setup -lboost_log -lboost_thread -lboost_regex -lboost_system
OPTIMIZE=-O2
DEPS=server.h connection.h config_parser.h request.h response.h request_handler.h echo_handler.h static_handler.h not_found_handler.h status_handler.h reverse_proxy_handler.h http_client.h sleep_handler.h markdown.h markdown-tokens.h markdown_handler.h
OBJ=server.o connection.o config_parser.o request.o response.o request_handler.o echo_handler.o static_handler.o not_found_handler.o status_handler.o reverse_proxy_handler.o http_client.o sleep_handler.o markdown.o markdown-tokens.o markdown_handler.o
GTEST_DIR=googletest/googletest
TESTS=config_parser_test connection_test server_test request_test echo_handler_test static_handler_test not_found_handler_test request_handler_test status_handler_test reverse_proxy_handler_test http_client_test response_test
DEPLOYS=deploy binary.tar webserver-image
PRIVATE_KEY_LOC=~/team22-ec2-key-pair.pem
EC2_HOST=ec2-54-218-71-128.us-west-2.compute.amazonaws.com
EC2_SERVER=ec2-user@ec2-54-218-71-128.us-west-2.compute.amazonaws.com

default: webserver

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $(OPTIMIZE) $(COV) -c -o $@ $<

webserver: main.cc $(OBJ)
	$(CXX) $(CXXFLAGS) $(OPTIMIZE) $(COV) -o $@ $^ $(LDLIBS)

libgtest.a:
	$(CXX) -std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

%_test: libgtest.a %_test.cc $(OBJ)
	$(CXX) $(CXXFLAGS) -isystem ${GTEST_DIR}/include $(COV) -pthread ${GTEST_DIR}/src/gtest_main.cc $^ -o $@ $(LDLIBS)

build-tests: $(TESTS)

test: integration-test unit-test

unit-test: build-tests
	for TEST in $(TESTS); do ./$$TEST || exit 1 ; done

integration-test: webserver
	./integration_tests.sh

cov-%: COV += -fprofile-arcs -ftest-coverage -g
cov-%: OPTIMIZE = -O0
cov-test: test lcov
cov-webserver: webserver
cov-integration: webserver integration-test lcov
cov-unit: unit-test lcov

lcov:
	lcov --capture --directory ./ --output-file coverage.info --no-external --base-directory ./ --quiet
	lcov --remove coverage.info '*/googletest/*' -o coverage.info
	genhtml coverage.info --output-directory covhtml
	lcov --list coverage.info
	printf "see covhtml/index.html for more information"

.PHONY: deploy
deploy:
	docker build -t webserver.build .
	docker run webserver.build > binary.tar
	mkdir -p deploy
	tar -xf binary.tar -C deploy
	rm binary.tar
	cp config deploy
	cp -r static deploy
	printf 'FROM busybox:ubuntu-14.04\n\nWORKDIR /opt/webserver\nCOPY . /opt/webserver\n\nCMD ["./webserver", "config", "-d"]' > deploy/Dockerfile
	docker build -t webserver deploy
	docker save -o webserver-image webserver
	scp -i $(PRIVATE_KEY_LOC) webserver-image $(EC2_SERVER):~
	rm webserver-image
	ssh -i $(PRIVATE_KEY_LOC) $(EC2_SERVER) 'docker load -i webserver-image'

run-deployed:
	ssh -i $(PRIVATE_KEY_LOC) $(EC2_SERVER) 'docker run -t -p 80:8080 webserver >> log 2>&1'

kill-deployed:
	ssh -i $(PRIVATE_KEY_LOC) $(EC2_SERVER) 'docker kill `docker ps -q`'

test-deployed:
	curl $(EC2_HOST)/echo

chrome-deployed:
	google-chrome $(EC2_HOST)/proxy1

login:
	ssh -i ~/team22-ec2-key-pair.pem $(EC2_SERVER)

clean:
	rm -rf $(OBJ) webserver $(TESTS) *.o *.gcda *.gcno *.gcov coverage.info *.a $(DEPLOYS) config_temp*
