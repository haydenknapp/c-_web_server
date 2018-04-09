#include <cassert>
#include <functional>
#include <string>

using std::string, std::function;

#include "Server.h"

string returnString(ServerFunctionParams params) {
	return "Hello!";
}

string anotherServerFunction(ServerFunctionParams params) {
	return "This is another!";
}

int main(int argc, char *argv[]) {
	assert(argc > 1);
	functionMap_t functionMap;
	functionMap["/simple"] = returnString;
	functionMap["/another"] = anotherServerFunction;
	Server server(atoi(argv[1]), 100, functionMap);
}
