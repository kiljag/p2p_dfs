
/*
cmd : ./dnode hub_port dnode_port rfc_port
listens for commands or messages from an hub via hub_port
listens for data transfer requests from other data nodes via dnode_port
listens for rfc commands via rfc_port
**/
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "common/dnode/rpc_server.h"

using namespace std;


// server process 1 (listens for hub requests)
void handle_hub_server(int hub_port) {
    /*TO DO :  incase a hub wants to initiate message connection with a dnode*/
    std::exit(0);
}


// server process 2 (listens for dnode data requests)
void handle_dnode_server(int dnode_port) {
    return;
}


int main(int argc, char* argv[]) {

    if (argc < 4) {
        std::cout << "not enough arguments!!" << endl;
        std::exit(1);
    }

    int hub_port = atoi(argv[1]);
    int dnode_port = atoi(argv[2]);
    int rpc_port = atoi(argv[3]);

    // if (fork() == 0) {
    //     handle_hub_server(hub_port);
    // }

    // if (fork() == 0) {
    //     handle_dnode_server(dnode_port);
    // }


    pid_t rpc_server_pid = fork();
    if (rpc_server_pid == 0) {
        handle_rpc_server(rpc_port);
    }

    int wstatus;
    pid_t child_pid = wait(&wstatus);
    std::cout << "child pid : " << child_pid << endl;

    std::exit(0);
}