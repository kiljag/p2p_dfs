#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "dnode/dnode.h"
#include "util/net.h"

using namespace std;

/*
cmd :: ./rfc hostip:port -u file_path
cmd :: ./rfc hostip:port -d file_name
*/

int main(int argc, char* argv[]) {

    if (argc < 4) {
        printf("insufficient arguments..\n");
        exit(0);
    }

    std::cout << "dnode ip:port ::" << argv[1] << std::endl;
    std::cout << "command :: " << argv[2] << std::endl;
    std::cout << "file path/name :: " << argv[3] << std::endl;
    
    struct in_addr dnode_ip_addr = parse_ip_addr(argv[1]);
    short dnode_port = parse_port(argv[1]);


    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("failed to create socket\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;

    // fill dnode server details
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr, &dnode_ip_addr, sizeof(dnode_ip_addr));
    serv_addr.sin_port = htons(dnode_port);


    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("unable to connect to server..\n");
        exit(0);
    }
    std::cout << "connected to dnode " << std::endl;

    char buffer[1024];
    struct rpc_cmd_struct rpc_cmd;
    
    
    if (string(argv[2]) == "-u") {
        rpc_cmd.cmd_type = RPC_COMMAND_UPLOAD;
        rpc_cmd.payload_len = strlen(argv[3]);

    } else if (string(argv[2]) == "-d") {
        rpc_cmd.cmd_type = RPC_COMMAND_DOWNLOAD;
        rpc_cmd.payload_len = strlen(argv[3]);

    } else {
        printf("Invalid command, %s\n", argv[2]);
        exit(0);
    }
    
    memcpy(buffer, argv[3], rpc_cmd.payload_len);

    std::cout << "sending rpc request " << std::endl;
    send(sockfd, &rpc_cmd, sizeof(rpc_cmd), 0);
    send(sockfd, buffer, rpc_cmd.payload_len + 1, 0);
    
    return 0;
}