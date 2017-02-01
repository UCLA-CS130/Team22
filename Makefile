CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
BOOSTFLAG = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o main.o
GTEST_DIR=googletest/googletest
TESTS=config_parser_test connection_test server_test

%.o: %.cc $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(BOOSTFLAG)

gtest-all.o:
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

build-tests: gtest-all.o
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -pthread config_parser_test.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o config_parser_test
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -pthread connection_test.cc connection.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o connection_test -lboost_system
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -pthread server_test.cc server.cc connection.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o server_test -lboost_system

test:
	make integration-test
	make unit-test

unit-test: build-tests
	./config_parser_test
	./connection_test
	./server_test

integration-test:
	make
	./integration_tests.sh

clean:
	rm -f $(OBJ) webserver $(TESTS) *.o
