
#include <iostream>
#include <string>
#include <map>

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "hub.h"


using namespace std;

/*
./hub root_dir hub_port
*/

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Insufficient arguments" << std::endl;
        std::exit(0);
    }

    char* hub_dir = argv[1];
    int hub_port = std::stoi(argv[2]);

    
    return 0;
}