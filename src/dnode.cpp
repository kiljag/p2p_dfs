
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
#include "dnode/data_server.h"

#include "hub/hub.h"
#include "util/net.h"

using namespace std;

struct dnode_details_struct dnode_details;




void handle_node_join() {

    // create a socket to contact hub
    int hub_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (hub_sockfd < 0) {
		perror("Unable to create hub socket\n");
		exit(0);
	}
    
    struct sockaddr_in hub_addr;
    hub_addr.sin_family	= AF_INET;
    inet_aton("127.0.0.1", &hub_addr.sin_addr);
    // hub_addr.sin_addr = dnode_details.hub_ip;
    hub_addr.sin_port = htons(dnode_details.hub_port);
    
    // connect to hub
	if (connect(hub_sockfd, (struct sockaddr *)&hub_addr, sizeof(hub_addr)) < 0) {
        perror("Unable to connect to hub!!");
        exit(0);
    }

    struct hub_cmd_struct hub_cmd;
    hub_cmd.cmd_type = NODE_JOIN;

    struct node_join_req_struct node_join_req;
    node_join_req.ip = dnode_details.dnode_ip;
    node_join_req.port = dnode_details.dnode_data_port;
    node_join_req.flags = 0;

    send(hub_sockfd, &hub_cmd, sizeof(hub_cmd), 0);
    send(hub_sockfd, &node_join_req, sizeof(node_join_req), 0);

    struct node_join_res_struct node_join_res;
    recv(hub_sockfd, &node_join_res, sizeof(node_join_res), 0);

    // save uid given hub
    dnode_details.uid = node_join_res.uid;

    close(hub_sockfd);
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

    std::cout << "dnode dir :: " << argv[1] << std::endl;
    std::cout << "hub ip:port :: " << argv[2] << std::endl;
    std::cout << "hub cmd port :: " << argv[3] << std::endl;
    std::cout << "dnode data port :: " << argv[4] << std::endl;
    std::cout << "rpc cmd port :: " << argv[5] << std::endl; 


    string dnode_root_dir = string(argv[1]);
    string dnode_files_dir = dnode_root_dir + string("/files");
    string dnode_meta_dir = dnode_root_dir + string("/meta");

    std::cout << "dnode_root_dir  :: " << dnode_root_dir << std::endl;
    std::cout << "dnode_files_dir :: " << dnode_files_dir << std::endl;
    std::cout << "dnode_meta_dir  :: " << dnode_meta_dir << std::endl;

    // store hub details
    dnode_details.hub_ip = parse_ip_addr(argv[2]);
    dnode_details.hub_port = parse_port(argv[2]);
    
    /*todo : should fill this properly*/
    inet_aton("localhost", &(dnode_details.dnode_ip));

    // store port details
    dnode_details.hub_cmd_port = (short)atoi(argv[3]);
    dnode_details.dnode_data_port = (short)atoi(argv[4]);
    dnode_details.rpc_port = (short)atoi(argv[5]);

    // store folders
    memcpy(dnode_details.root_dir, dnode_root_dir.c_str(), dnode_root_dir.size()+1);
    memcpy(dnode_details.files_dir, dnode_files_dir.c_str(), dnode_files_dir.size() + 1);
    memcpy(dnode_details.meta_dir, dnode_meta_dir.c_str(), dnode_meta_dir.size() + 1);

    // in case the node is joining for first time
    handle_node_join();
    std::cout << "dnode uid :: " << dnode_details.uid << std::endl; 
    
    // char dnode_buf[2048];

    // if (fork() == 0) {
    //     handle_hub_server(hub_port);
    // }

    pid_t data_server_port = fork();
    if (data_server_port == 0) {
        handle_data_server(&dnode_details);
    }


    pid_t rpc_server_pid = fork();
    if (rpc_server_pid == 0) {
        handle_rpc_server(&dnode_details);
    }

    int wstatus;
    pid_t child_pid = wait(&wstatus);
    std::cout << "child pid : " << child_pid << endl;

    std::exit(0);
}