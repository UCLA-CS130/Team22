CC = g++
CFLAGS = -I. -lboost_system
CXXFLAGS = -std=c++11
DEPS = server.h connection.h
OBJ = server.o connection.o main.o

%.o: %.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $< $(CFLAGS)

webserver: $(OBJ)
	g++ $(CXXFLAGS) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ)