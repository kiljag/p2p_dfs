
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

// filehash, list_of_dnode_uids mapping
map<uint64_t, vector<uint64_t>> fhash_map;

// filename, filehash mapping
map<string, uint64_t> fname_map;

// to store dnode_details
map<uint16_t, struct dnode_struct *> dnodes_map;


void handle_new_node_join(int dnode_sockfd, struct node_join_req_struct *node_join_req) {

    // generate random id for the dnode
    uint64_t dnode_id = ((uint64_t)rand() << 32) ^ (uint64_t)(rand());
    printf("new node joined : uid : %08lx\n", dnode_id);

    // fill up dnode details
    struct dnode_struct *dnode_details = (struct dnode_struct *)malloc(sizeof(struct dnode_struct));
    dnode_details->uid = dnode_id;
    dnode_details->ip = node_join_req->ip;
    dnode_details->port = node_join_req->port;
    dnode_details->flags = 0; /*TODO :*/

    // save dnode in memory
    dnodes_map[dnode_id] = dnode_details;

    // send result back to dnode
    struct node_join_res_struct node_join_res;
    node_join_res.uid = dnode_id;
    send(dnode_sockfd, &node_join_res, sizeof(node_join_res), 0);

    return;
}

void handle_file_upload_req(int dnode_fd, struct file_upload_req_struct * file_upload_req) {
    
    // parse file upload request
    uint64_t dnode_id  = file_upload_req->data_node_uid;
    uint64_t fhash = file_upload_req->file_hash;
    string fname = string(file_upload_req->file_name);

    // if the fhash entry is absent
    if (fhash_map.find(fhash) == fhash_map.end()) {
        fhash_map[fhash] = vector<uint64_t>();
    }
    fhash_map[fhash].push_back(dnode_id);
            
    // add the fname entry
    if (fname_map.find(fname) == fname_map.end()) {
        fname_map[fname] = fhash;
    } 

    /*TODO: should send something back to dnode*/

    return;
}

void handle_file_download_req(int dnode_sockfd, struct file_download_req_struct * file_download_req) {

    // parse file download request
    string fname =  string(file_download_req->file_name);
    uint64_t fhash =  fname_map[fname];

    // intialize file_index_data // todo

    // fetch dnodes which contain the file with fhash
    vector<uint64_t> dnode_ids = fhash_map[fhash];
    int num_peer_nodes = dnode_ids.size();

    // initialize peer_dnodes_list
    struct peer_dnode_struct * peer_dnodes_list;
    int peer_dnodes_list_size = sizeof(struct peer_dnode_struct) * num_peer_nodes;
    peer_dnodes_list = (struct peer_dnode_struct *)malloc(peer_dnodes_list_size);

    for(int i=0; i < dnode_ids.size(); i++) {
        struct dnode_struct *dnode = dnodes_map[dnode_ids[i]];
        peer_dnodes_list[i].ip = dnode->ip;
        peer_dnodes_list[i].port = dnode->port;
        peer_dnodes_list[i].flags = 0;
    }

    // fill file download response
    struct file_download_res_struct file_download_res;
    file_download_res.file_hash = fhash;        
    file_download_res.num_peer_dnodes = num_peer_nodes;
    file_download_res.file_index_data_len = 0;

    // send response
    send(dnode_sockfd, &file_download_res, sizeof(file_download_res), 0);

    //send file index data and peer nodes data
    send(dnode_sockfd, peer_dnodes_list, peer_dnodes_list_size, 0);

    return;
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
        // recv(newsockfd, buffer, 2048, 0);
        // uint16_t command;
        // memcpy(&command, buffer, sizeof(command));
        struct hub_cmd_struct hub_cmd;
        recv(newsockfd, &hub_cmd, sizeof(hub_cmd), 0);

        if(hub_cmd.cmd_type == NODE_JOIN) {

            std::cout << "Hub :: Node join" << std::endl;
            struct node_join_req_struct node_join_req;
            recv(newsockfd, &node_join_req, sizeof(node_join_req), 0);

            handle_new_node_join(newsockfd, &node_join_req);
        } 

        else if (hub_cmd.cmd_type == FILE_UPLOAD) {
            
            std::cout << "Hub :: File upload" << std::endl;
            struct file_upload_req_struct file_upload_req;
            recv(newsockfd, &file_upload_req, sizeof(file_upload_req), 0);
            
            handle_file_upload_req(newsockfd, &file_upload_req);
        }

        else if (hub_cmd.cmd_type == FILE_DOWNLOAD) {

            std::cout << "Hub :: File download request" << std::endl;
            struct file_download_req_struct file_download_req;
            recv(newsockfd, &file_download_req, sizeof(file_download_req), 0);

            handle_file_download_req(newsockfd, &file_download_req);
        }

        close(newsockfd);

    }

    close(sockfd);
    
    return 0;
}