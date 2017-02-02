CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
OPTIMIZE=-O2
BOOSTFLAG = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o
GTEST_DIR=googletest/googletest
TESTS=config_parser_test connection_test server_test

default: webserver
	
%.o: %.cc $(DEPS)
	$(CC) $(CXXFLAGS) $(OPTIMIZE) $(COV) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) $(OPTIMIZE) $(COV) -o $@ $^ main.cc $(BOOSTFLAG)

libgtest.a:
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

build-tests: libgtest.a $(OBJ)
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread config_parser_test.cc $(OBJ) ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o config_parser_test -lboost_system
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread connection_test.cc $(OBJ) ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o connection_test -lboost_system
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread server_test.cc $(OBJ) ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o server_test -lboost_system

test:
	make integration-test
	make unit-test

unit-test: build-tests
	./config_parser_test
	./connection_test
	./server_test

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

integration-test: webserver
	./integration_tests.sh

clean:
	rm -f $(OBJ) webserver $(TESTS) *.o *.gcda *.gcno *.gcov coverage.info *.a
