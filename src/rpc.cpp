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


    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd < 0){
    //     perror("failed to create socket\n");
    //     exit(0);
    // }

    // struct sockaddr_in serv_addr;

    // // fill dnode server details
    // serv_addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1", &serv_addr.sin_addr);
    // // memcpy(&serv_addr.sin_addr, &dnode_ip_addr, sizeof(dnode_ip_addr));
    // serv_addr.sin_port = htons(dnode_port);

    

    
    // if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    //     perror("unable to connect to server..\n");
    //     exit(0);
    // }

    std::cout << "RPC :: connecting to dnode.. " << std::endl;
    int dnode_sockfd = connect_to_server(dnode_ip_addr, dnode_port);
    std::cout << "RPC :: connected to dnode.." << std::endl;

    char buffer[1024];
    struct rpc_req_struct rpc_req;
    char* payload = argv[3];
    int payload_len = strlen(payload) + 1; //to include null byte

    if (string(argv[2]) == "-u") {
        rpc_req.req_type = RPC_REQ_UPLOAD;
        rpc_req.payload_len = payload_len;

    } else if (string(argv[2]) == "-d") {
        rpc_req.req_type = RPC_REQ_DOWNLOAD;
        rpc_req.payload_len = payload_len;

    } else {
        printf("Invalid command, %s\n", argv[2]);
        exit(0);
    }


    std::cout << "RPC :: sending rpc request " << std::endl;
    send_full(dnode_sockfd, &rpc_req, sizeof(rpc_req));

    std::cout << "RPC :: sending payload " << std::endl;
    send_full(dnode_sockfd, payload, payload_len);

    // read response from the dnode server
    struct rpc_res_struct rpc_res;
    recv_full(dnode_sockfd, &rpc_res, sizeof(rpc_res));

    if (rpc_res.res_type == RPC_RES_SUCCEES) {
        std::cout << "RPC :: op success !!" << std::endl;

    } else if (rpc_res.res_type == RPC_RES_FAILURE) {
        std::cout << "RPC :: op failure !!" << std::endl;
    
    } else {
        std::cout << "Invalid response type" << std::endl;
    }

    //close connection
    disconnect_from_server(dnode_sockfd);

    return 0;
}