
#include <iostream>
#include <string>
#include <vector>
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

#include "hub/hub.h"
#include "util/net.h"


using namespace std;

// to store file_hash, node_id mappings
map<uint64_t, vector<uint64_t>> fhash_map;

// to store dnode_details
vector<struct dnode_struct *> dnode_list;

void add_dnode_details(struct node_join_struct *node_join_details) {

    struct dnode_struct *dnode_details = (struct dnode_struct *)malloc(sizeof(struct dnode_struct));
    
    dnode_details->uid = node_join_details->uid;
    dnode_details->ip = node_join_details->ip;
    dnode_details->port = node_join_details->port;
    dnode_details->flags = 0;

    dnode_list.push_back(dnode_details);
}

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

    int sockfd = create_server(hub_port);
    
    char buffer[2048]; // multi-purpose hub buffer

    if (listen(sockfd, 5) == 0) {
        std::cout << "Hub :: listening for dnode requests on " << hub_port << std::endl;
    }

    
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    while(1) {

        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("Accept error\n");
            std::exit(0);
        }

        std::cout << "Hub :: Handling request" << std::endl;
        recv(newsockfd, buffer, 2048, 0);
        uint16_t command;
        memcpy(&command, buffer, sizeof(command));

        if(command == NODE_JOIN) {

            std::cout << "Hub :: Node join" << std::endl;
            struct node_join_struct node_join_details;
            memcpy(&node_join_details, buffer+2, sizeof(struct node_join_struct));
            add_dnode_details(&node_join_details);
            printf("new node joined : uid : %08x\n", node_join_details.flags);

        } else if (command == FILE_UPLOAD) {

            std::cout << "Hub :: File upload" << std::endl;
            struct file_upload_struct file_upload_details;
            memcpy(&file_upload_details, buffer+2, sizeof(struct file_upload_struct));
            uint64_t dnode_id  = file_upload_details.data_node_uid;
            uint64_t fhash = file_upload_details.file_hash;
            
            // if the fhash entry is absent
            if (fhash_map.find(fhash) == fhash_map.end()) {
                fhash_map[fhash] = vector<uint64_t>();
                fhash_map[fhash].push_back(dnode_id);
            }

        } else if (command == FILE_DOWNLOAD) {

            struct file_download_struct file_download_details;
            memcpy(&file_download_details, buffer+2, sizeof(struct file_download_struct));

        }



    }
    
    return 0;
}