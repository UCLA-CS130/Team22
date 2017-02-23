CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
OPTIMIZE=-O2
BOOSTFLAG = -lboost_system -lboost_regex
DEPS=server.h connection.h config_parser.h request.h response.h request_handler.h echo_handler.h static_handler.h not_found_handler.h status_handler.h
OBJ=server.o connection.o config_parser.o request.o response.o request_handler.o echo_handler.o static_handler.o not_found_handler.o status_handler.o
GTEST_DIR=googletest/googletest
TESTS=config_parser_test connection_test server_test request_test echo_handler_test static_handler_test not_found_handler_test status_handler_test

default: webserver

%.o: %.cc $(DEPS)
	$(CC) $(CXXFLAGS) $(OPTIMIZE) $(COV) -c -o $@ $<

webserver: main.cc $(OBJ)
	g++ $(CXXFLAGS) $(OPTIMIZE) $(COV) -o $@ $^ $(BOOSTFLAG)

libgtest.a:
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

%_test: libgtest.a %_test.cc $(OBJ)
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread ${GTEST_DIR}/src/gtest_main.cc $^ -o $@ $(BOOSTFLAG)	

build-tests: $(TESTS)

test: integration-test unit-test

unit-test: build-tests
	for TEST in $(TESTS); do ./$$TEST ; done

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

clean:
	rm -f $(OBJ) webserver $(TESTS) *.o *.gcda *.gcno *.gcov coverage.info *.a
