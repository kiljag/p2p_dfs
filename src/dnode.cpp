
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

#include <string>

#include "dnode/dnode.h"
#include "dnode/rpc_server.h"
#include "hub/hub.h"

using namespace std;

struct dnode_details node_details;


// server process 1 (listens for hub requests)
void handle_hub_server(int hub_port) {
    /*TO DO :  incase a hub wants to initiate message connection with a dnode*/
    std::exit(0);
}


// server process 2 (listens for dnode data requests)
void handle_dnode_server(int dnode_port) {
    return;
}


/*
cmd : ./dnode dnode_dir hub_ip:port hub_port dnode_port rfc_port
*/

int main(int argc, char* argv[]) {

    if (argc < 6) {
        std::cout << "not enough arguments!!" << endl;
        std::exit(1);
    }

    char* dnode_dir = argv[1];

    string hub_ip_port(argv[2]);
    int del_pos = hub_ip_port.find(":");
    string hub_ip_str = hub_ip_port.substr(0, del_pos);
    string hub_port_str = hub_ip_port.substr(del_pos+1, hub_ip_port.size());

    inet_aton("localhost", &(node_details.dnode_ip));
    inet_aton(hub_ip_str.c_str(), &(node_details.hub_ip));
    node_details.hub_port = stoi(hub_port_str);

    node_details.hub_cmd_port = atoi(argv[3]);
    node_details.dnode_data_port = atoi(argv[4]);
    node_details.rpc_port = atoi(argv[5]);

    string root_dir(dnode_dir);
    string files_dir = dnode_dir + string("/files");

    // fill up node details
    node_details.uid = (uint64_t)rand();
    node_details.root_dir = (char *)calloc(root_dir.size()+1, sizeof(char));
    node_details.files_dir = (char *)calloc(files_dir.size() + 1, sizeof(char));

    memcpy(node_details.root_dir, root_dir.c_str(), root_dir.size());
    memcpy(node_details.files_dir, files_dir.c_str(), files_dir.size());

    std::cout << "root dir : " << node_details.root_dir << endl;
    std::cout << "files dir : " << node_details.files_dir << endl;

    char dnode_buf[2048];

    // send a join request to hub
    int hub_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (hub_sockfd < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
    
    struct sockaddr_in hub_addr;
    hub_addr.sin_family	= AF_INET;
    hub_addr.sin_addr = node_details.hub_ip;
    hub_addr.sin_port = htons((short)node_details.hub_port);
    
	if (connect(hub_sockfd, (struct sockaddr *)&hub_addr, sizeof(hub_addr)) < 0) {
        perror("Unable to connect to hub!!");
        exit(0);
    }

    struct node_join_struct node_join_details;
    node_join_details.uid = node_details.uid;
    node_join_details.ip = node_details.dnode_ip;
    node_join_details.port = node_details.dnode_data_port;
    node_join_details.flags = 0;

    uint16_t command  = NODE_JOIN;
    memcpy(dnode_buf, &command, sizeof(command));
    memcpy(dnode_buf + 2, &node_join_details, sizeof(node_join_details));
    int payload_len = 2 + sizeof(node_join_details);
    send(hub_sockfd, dnode_buf, payload_len, 0);
    close(hub_sockfd);

    
    
    // if (fork() == 0) {
    //     handle_hub_server(hub_port);
    // }

    // if (fork() == 0) {
    //     handle_dnode_server(dnode_port);
    // }


    pid_t rpc_server_pid = fork();
    if (rpc_server_pid == 0) {
        handle_rpc_server(&node_details);
    }

    int wstatus;
    pid_t child_pid = wait(&wstatus);
    std::cout << "child pid : " << child_pid << endl;

    std::exit(0);
}