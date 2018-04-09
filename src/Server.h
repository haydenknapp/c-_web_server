#pragma once

#include <vector>
#include <cassert>
#include <list>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <iostream>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

using std::unordered_map;
using std::function;
using std::string;
using std::vector;
using std::generate;
using std::cout;
using std::endl;
using std::find;


enum class RequestTypes { get, post };
typedef int RequestTypes_t;

struct ServerFunctionParams {
	RequestTypes_t requestType;
};

struct ConnectionParams {
	pthread_t thread;
	int client_socket;
	bool inUse;
};

void *handleClientCon(ConnectionParams *);

class Server {
public:
	Server(int, int);
private:
	typedef unordered_map<string, function<void(ServerFunctionParams)>> functionMap_t;
	functionMap_t routeNamesFunctions;
	vector<ConnectionParams> conThreads;
};

Server::Server(int port, int maxCons) {
	conThreads = vector<ConnectionParams>(maxCons);
	generate(conThreads.begin(), conThreads.end(), 
			[] () { ConnectionParams param; param.inUse = false; return param; } );

	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	listen(server_socket, 5);

	char client_request[2048];
	int client_socket;

	char informational[] = "HTTP/1.1 102 OK\r\n\n";
	char http_header[] = "HTTP/1.1 200 OK\r\n\n\
		<html><body>Hello!</body></html>";

	while (true) {
		client_socket = accept(server_socket, NULL, NULL);
		recv(client_socket, &client_request, sizeof(client_request), 0);
		send(client_socket, informational, sizeof(informational), 0);

		auto openCon = find_if(conThreads.begin(), conThreads.end(),
				[](auto a) { return !a.inUse; });

		assert(openCon != conThreads.end());
		openCon->inUse = true;
		openCon->client_socket = client_socket;
		pthread_create(&openCon->thread, NULL, (void *(*)(void *)) handleClientCon, &*openCon);
		//send(client_socket, http_header, sizeof(http_header), 0);
		//close(client_socket);
	}
}

void *handleClientCon(ConnectionParams *params) {
	int client_socket = params->client_socket;
	pthread_t thread = params->thread;

	char http_header[] = "HTTP/1.1 200 OK\r\n\n\
		<html><body>Hi!</body></html>";
	sleep(3);
	send(client_socket, http_header, sizeof(http_header), 0);

	close(client_socket);
	pthread_join(thread, NULL);
}
