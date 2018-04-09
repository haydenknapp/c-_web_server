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

typedef unordered_map<string, function<string(ServerFunctionParams)>> functionMap_t;

struct ConnectionParams {
	pthread_t thread;
	int client_socket;
	string request;
	bool inUse;
	functionMap_t functionMap;
};


void *handleClientCon(ConnectionParams *);

class Server {
public:
	function<string(ServerFunctionParams)> sample;

	Server(int, int, functionMap_t);
private:
	functionMap_t routeNamesFunctions;
	vector<ConnectionParams> conThreads;
};

Server::Server(int port, int maxCons, functionMap_t functionMap) {
	conThreads = vector<ConnectionParams>(maxCons);
	routeNamesFunctions = functionMap;
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

		openCon->inUse = true;
		openCon->client_socket = client_socket;
		openCon->request = client_request;
		openCon->functionMap = routeNamesFunctions;
		pthread_create(&openCon->thread, NULL, (void *(*)(void *)) handleClientCon, &*openCon);
	}
}

void *handleClientCon(ConnectionParams *params) {
	int client_socket = params->client_socket;
	pthread_t thread = params->thread;
	functionMap_t functionMap = params->functionMap;
	string request = params->request;

	ServerFunctionParams param;

	char http_header[] = "HTTP/1.1 200 OK\r\n\n";

	string type = request.substr(0, request.find(' '));
	string route = request.substr(request.find(' ') + 1, request.find(' ', request.find(' ') + 1) - request.find(' ') - 1);
	
	string ret;
	if (functionMap.find(route) != functionMap.end())
		ret = http_header + functionMap[route](param);
	else
		ret = http_header + string("404");

	send(client_socket, ret.c_str(), ret.size(), 0);

	close(client_socket);
	pthread_join(thread, NULL);
}
