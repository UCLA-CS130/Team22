CXX=g++
CXXFLAGS=-std=c++11 -I. -Wall -Werror
BOOSTFLAG = -lboost_system
DEPS=server.h connection.h config_parser.h
OBJ=server.o connection.o config_parser.o main.o
TESTOBJ=server.gcno connection.gcno config_parser.gcno main.gcno

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(BOOSTFLAG)

test:
	./build_tests.sh
	./config_parser_test

coverage: CXXFLAGS += -fprofile-arcs -ftest-coverage -g
coverage: webserver
	
cov: foo.cc
	g++ -fprofile-arcs -ftest-coverage -g -O0 -Wall foo.cc -o foo
	./foo
	gcov -r foo.cc
	
lcov: cov
	lcov --capture --directory ./ --output-file coverage.info
	genhtml coverage.info --output-directory coverage
	
test-curl: 
	curl http://localhost:8080

integration-test:
	make
	./integration_tests.sh
	
clean:
	rm -f $(OBJ) $(TESTOBJ) webserver *.gcda *.gcno *.gcov
	rm -rf cov/*
	rm -f foo
