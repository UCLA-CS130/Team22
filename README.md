# Team22 - CS130 Web Server
**Members: Justin Teo, Kevin Zhu, David Stephan**

## Simple web server implementation.

### Usage
To properly clone repository to own maching, run the following command:
```
git clone --recursive https://github.com/UCLA-CS130/Team22.git
```
Edit `config` file to specify the port for the server to run on along with handlers that are supported.

Run server using:
```
$ make
$ ./webserver config
```
The server runs by default on an info logging level, but the following options can specify the severity level to print:
* `-d` debugging mode which prints out all statements including trace and debugging
* `-s` surpress mode which prevents all logging statements

The server can be accessed with any method that can work over HTTP such as the following:
* Use your browser to view the page: `http://localhost:8080/`
* Use a curl request: `curl localhost:8080`

The following paths are currently configured:
* / = simple reverse proxy request to the UCLA website
* /echo = echo server that echos back the request
* /special/city.jpg = loads jpg image with static handler
* /status = status page for statistics on requests to server

### Make Commands
* `make` builds the server
* `make test` builds all test files and runs both the integration test and all unit tests
* `make unit-test` builds and runs only unit tests
* `make integration-test` builds and runs only integration tests
* `make cov-test` runs coverage test; run a `make clean` beforehand for more accurate results
* `make deploy` builds a docker image and sends it to the currently configured ec2 server; needs private pem key in home directory to work
* `make run-deployed` connects to the ec2 server and runs the docker image installed there
* `make kill-deployed` kills any running docker process on the ec2 server

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
* `status_handler.cc`: A class which inherits from RequestHandler to show statistics and status of server
* `reverse_proxy_handler.cc`: A class which inherits from RequestHandler to load up another site as a reverse proxy
* `sleep_handler.cc`: A class which inherits from RequestHandler that sleeps to test multithreading
* `http_client.cc`: A class which helps open an HTTP connection to another site
