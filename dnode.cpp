
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

using namespace std;

int create_server(int port) {

    int sockfd;
    struct sockaddr_in servaddr;

    // socket creation
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("socket creation failed..");
        exit(EXIT_FAILURE);
    }

    // assign ip, port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    //bind to port
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("failed to bind to server port");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}


// server process 1 (listens for hub requests)
void handle_hub_server(int hub_port) {
    /*TO DO :  incase a hub wants to initiate message connection with a dnode*/
    exit(0);
}


// server process 2 (listens for dnode data requests)
void handle_dnode_server(int dnode_port) {
    return;
}

// server process 3 (listens for rfc requests)
void handle_rfc_server(int rfc_port) {

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {

    if (argc < 4) {
        cout << "not enough arguments!!" << endl;
        exit(1);
    }

    int hub_port = atoi(argv[1]);
    int dnode_port = atoi(argv[2]);
    int rfc_port = atoi(argv[3]);

    // if (fork() == 0) {
    //     handle_hub_server(hub_port);
    // }

    // if (fork() == 0) {
    //     handle_dnode_server(dnode_port);
    // }

    pid_t rfc_server_pid = fork();
    if (rfc_server_pid == 0) {
        handle_rfc_server(rfc_port);
    }

    int wstatus;
    pid_t child_pid = wait(&wstatus);
    cout << "child pid : " << child_pid << endl;

    exit(0);
}