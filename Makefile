CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
OPTIMIZE=-O0
BOOSTFLAG = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o main.o
GTEST_DIR=googletest/googletest
TESTS=config_parser_test connection_test server_test

default: webserver
	
%.o: %.cc $(DEPS)
	$(CC) $(CXXFLAGS) $(OPTIMIZE) $(COV) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) $(COV) -o $@ $^ $(BOOSTFLAG)

gtest-all.o:
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

build-tests: gtest-all.o
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread config_parser_test.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o config_parser_test
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread connection_test.cc connection.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o connection_test -lboost_system
	g++ -std=c++0x -isystem ${GTEST_DIR}/include $(COV) -pthread server_test.cc server.cc connection.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o server_test -lboost_system

test: build-tests
	./config_parser_test
	./connection_test
	./server_test

cov-%: COV += -fprofile-arcs -ftest-coverage -g
cov-test: test lcov
cov-integration: webserver integration-test lcov

cov: foo.cc
	g++ -fprofile-arcs -ftest-coverage -g -O0 -Wall foo.cc -o foo
	./foo
	gcov -r foo.cc
	
lcov:
	lcov --capture --directory ./ --output-file coverage.info --no-external --base-directory ./ --quiet
	lcov --remove coverage.info '*/googletest/*' -o coverage.info
	genhtml coverage.info --output-directory covhtml
	lcov --list coverage.info
	printf "see covhtml/index.html for more information"
	
test-curl: 
	curl http://localhost:8080

integration-test: webserver
	./integration_tests.sh
	printf "hey"

clean:
	rm -f $(OBJ) webserver $(TESTS) *.o *.gcda *.gcno *.gcov
	rm -rf covhtml/*
