CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
OPTIMIZE=-O0
BOOSTFLAG = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o main.o
GTEST_DIR=googletest/googletest

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) $(OPTIMIZE) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(BOOSTFLAG)

test:
	./build_tests.sh
	./config_parser_test

coverage: CXXFLAGS += -fprofile-arcs -ftest-coverage -g
coverage: webserver
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	g++ -std=c++0x -isystem ${GTEST_DIR}/include -pthread config_parser_test.cc config_parser.cc -fprofile-arcs -ftest-coverage ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o config_parser_test
	./config_parser_test
	gcov -r config_parser_test.cc
	
cov: foo.cc
	g++ -fprofile-arcs -ftest-coverage -g -O0 -Wall foo.cc -o foo
	./foo
	gcov -r foo.cc
	
lcov:
	lcov --capture --directory ./ --output-file coverage.info --no-external --base-directory ./ --quiet
	lcov --remove coverage.info '*/googletest/*' -o coverage.info
	genhtml coverage.info --output-directory covhtml
	
test-curl: 
	curl http://localhost:8080

integration-test:
	make
	./integration_tests.sh
	
clean:
	rm -f $(OBJ) webserver *.gcda *.gcno *.gcov
	rm -rf covhtml/*
	rm -f foo
