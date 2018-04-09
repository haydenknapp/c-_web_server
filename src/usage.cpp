#include <cassert>

#include "Server.h"

int main(int argc, char *argv[]) {
	assert(argc > 1);
	Server server(atoi(argv[1]), 100);
}
