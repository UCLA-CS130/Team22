# Team22 - CS130 Web Server
**Members: Justin Teo, Kevin Zhu, David Stephan**
Simple web server implementation.

### Usage
Edit `config` to specify the port for the server to run on along with handlers that are supported.
Run server using:
```
$ make
$ ./webserver config
```
The server runs by default on an info logging level, but the option `-d` can be specified to run in debugging mode which prints out all debugging statements.

The server can be accessed with any method that can work over HTTP such as the following:
* Use your browser to view the page: `http://localhost:8080/`
* Use a curl request: `curl localhost:8080`

### Make Commands
* `make` builds the server
* `make test` builds all test files and runs both the integration test and all unit tests
* `make unit-test` builds and runs only unit tests
* `make integration-test` builds and runs only integration tests
* `make cov-test` runs coverage test; run a `make clean` beforehand for more accurate results

### Adding New Request Handlers
* First, modify the Makefile to add the new corresponding .h file to the `DEPS` variable and corresponding .o file to the `OBJ` variable. Add the test file name to the `TESTS` variable for `make test` to also run the new request handler's test.
* Create the .h and .cc file along with a \_test.cc file for unit tests. Inherit from the RequestHandler class and implement the pure virtual functions `Init` and `HandleRequest`.
* View the EchoHandler, StaticHandler, or NotFoundHandler for tips on how to structure the new class

### Source Code
* `main.cc`: Gets command line arguments and calls config parser to pass into Server class
* `config_parser.cc`: Parses the config file and tokenizes it
* `server.cc`: Processes config blocks to construct server and request handlers and listens on specified port for new connections
* `connection.cc`: Receives request and retrieves appropriate request handler to send back a response
* `request.cc`: A class that can assemble and store a HTTP request
* `response.cc`: A class that can assemble and store a HTTP response
* `request_handler.cc`: Base class which is basis for all other request handlers
* `echo_handler.cc`: A class which inherits from RequestHandler to echo back the request as a response
* `static_handler.cc`: A class which inherits from RequestHandler to serve up a static file from a directory on the server
* `not_found_handler.cc`: A class which inherits from RequestHandler to serve a 404 error page
