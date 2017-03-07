CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror -static-libgcc -static-libstdc++ -pthread -Wl,-Bstatic
LDLIBS= -lboost_log_setup -lboost_log -lboost_thread -lboost_regex -lboost_system
OPTIMIZE=-O2
DEPS=server.h connection.h config_parser.h request.h response.h request_handler.h echo_handler.h static_handler.h not_found_handler.h status_handler.h reverse_proxy_handler.h http_client.h sleep_handler.h
OBJ=server.o connection.o config_parser.o request.o response.o request_handler.o echo_handler.o static_handler.o not_found_handler.o status_handler.o reverse_proxy_handler.o http_client.o sleep_handler.o
GTEST_DIR=googletest/googletest
TESTS=config_parser_test connection_test server_test request_test echo_handler_test static_handler_test not_found_handler_test request_handler_test status_handler_test reverse_proxy_handler_test http_client_test response_test
DEPLOYS=deploy binary.tar webserver-image

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
	printf 'FROM busybox:ubuntu-14.04\n\nWORKDIR /opt/webserver\nCOPY . /opt/webserver\n\nEXPOSE 8080:8080\nCMD ["./webserver", "config"]' > deploy/Dockerfile
	docker build -t webserver deploy
	docker save -o webserver-image webserver
	scp -i ~/team22-ec2-key-pair.pem webserver-image ec2-user@ec2-54-218-71-128.us-west-2.compute.amazonaws.com:~
	rm webserver-image
	ssh -i ~/team22-ec2-key-pair.pem ec2-user@ec2-54-218-71-128.us-west-2.compute.amazonaws.com 'docker load -i webserver-image'

run-deployed:
	ssh -i ~/Downloads/team22-ec2-key-pair.pem ec2-user@ec2-54-218-71-128.us-west-2.compute.amazonaws.com 'docker run --rm -t -p 80:8080 webserver &'

kill-deployed:
	ssh -i ~/Downloads/team22-ec2-key-pair.pem ec2-user@ec2-54-218-71-128.us-west-2.compute.amazonaws.com 'docker kill `docker ps -q`'

clean:
	rm -rf $(OBJ) webserver $(TESTS) *.o *.gcda *.gcno *.gcov coverage.info *.a $(DEPLOYS)
