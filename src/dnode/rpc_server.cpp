#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "dnode.h"
#include "rpc_server.h"

#include "../hub/hub.h"
#include "../util/hash.h"
#include "../util/net.h"
#include "../util/fs.h"

void handle_file_upload(struct dnode_details_struct *dnode_details, int rpc_cli_fd, char *file_path) {

    int file_len = get_file_size(file_path);
    std::cout << "in_file_path : " << file_path << std::endl;
    std::cout << "file size : " << file_len << std::endl;

    string file_path_str(file_path);
    string file_name_str = file_path_str.substr(file_path_str.find_last_of("/") + 1, file_path_str.size());
    string out_path = string(dnode_details->files_dir) + string("/") + file_name_str;

    uint8_t *file_buff = (uint8_t *)malloc(file_len);
    size_t bytes_read = fread_full(file_path, file_buff, file_len);
    std::cout << "bytes read :: " << bytes_read << std::endl;

    // compute file hash
    struct fhash h = compute_hash(file_buff, file_len);
    // printf("%016lx%016lx%016lx%016lx\n", h.a, h.b, h.c, h.d);
    uint64_t file_hash = reduce_hash(&h);
    printf("file hash : %016lx\n", file_hash);

    // send the file hash to the hub
    struct hub_cmd_struct hub_cmd;
    hub_cmd.cmd_type = FILE_UPLOAD;

    // calculate the file index data /* TODO */

    // fill the file upload request details
    struct file_upload_req_struct file_upload_req;
    file_upload_req.data_node_uid = dnode_details->uid;
    file_upload_req.file_hash = file_hash;
    file_upload_req.file_index_data_len = 0;
    memcpy(file_upload_req.file_name, file_name_str.c_str(), file_name_str.size() + 1);
    file_upload_req.num_chunks = 0; // need to calculate 

    // connect to hub
    int hub_sockfd = connect_to_server(dnode_details->hub_ip, dnode_details->hub_port);
    std::cout << "uploading file metadata to hub.." << std::endl;
    send(hub_sockfd, &hub_cmd, sizeof(hub_cmd), 0);
    send(hub_sockfd, &file_upload_req, sizeof(file_upload_req), 0);

    // disconnect from hub
    disconnect_from_server(hub_sockfd);

    // save the file in dnode files directory
    std::cout << "out file patth " << out_path << std::endl;
    size_t bytes_written = fwrite_full(out_path.c_str(), file_buff, file_len);
    std::cout << "bytes written  :: " << bytes_written << std::endl;

    return;
}

void handle_file_download(struct dnode_details_struct *dnode_details, int rpc_cli_fd, char *file_name) {

    string file_name_str(file_name);
    string out_path = string(dnode_details->files_dir) + string("/") + file_name_str;

    struct hub_cmd_struct hub_cmd;
    hub_cmd.cmd_type = FILE_DOWNLOAD;

    // fill file download request details
    struct file_download_req_struct file_download_req;
    file_download_req.dnode_uid = dnode_details->uid;
    memcpy(file_download_req.file_name, file_name, strlen(file_name) + 1);

    // connect to hub
    int hub_sockfd = connect_to_server(dnode_details->hub_ip, dnode_details->hub_port);

    send(hub_sockfd, &hub_cmd, sizeof(hub_cmd), 0);
    send(hub_sockfd, &file_download_req, sizeof(file_download_req), 0);

    // recv file download response from server
    struct file_download_res_struct file_download_res;
    recv(hub_sockfd, &file_download_res, sizeof(file_download_res), 0);
    int num_peer_dnodes = file_download_res.num_peer_dnodes;
    int file_index_data_len = file_download_res.file_index_data_len;
    std::cout << "num peer dnodes : " << num_peer_dnodes << std::endl;

    // recv peer dnodes data and file index data from hub
    struct peer_dnode_struct * peer_dnode_list;
    peer_dnode_list = (struct peer_dnode_struct *)malloc(sizeof(struct peer_dnode_struct) * num_peer_dnodes);
    uint8_t *file_index_data = (uint8_t *)malloc(file_index_data_len);

    read_full(hub_sockfd, peer_dnode_list, sizeof(struct peer_dnode_struct) * num_peer_dnodes);
    if (file_index_data_len > 0) {
        read_full(hub_sockfd, file_index_data, file_index_data_len);
    }

    // retrieve first peer details
    struct in_addr peer_ip = peer_dnode_list[0].ip;
    short peer_port = peer_dnode_list[0].port;

    int file_hash = file_download_res.file_metadata.file_hash;
    int file_size = file_download_res.file_metadata.file_size;
    uint8_t* file_data = (uint8_t *)malloc(file_size);
    
    // connect to peer
    int peer_fd = connect_to_server(peer_ip, peer_port);

    // fill data request details
    file_data_req_struct file_data_req;
    file_data_req.file_hash = file_hash;
    memcpy(file_data_req.file_name, file_name, strlen(file_name) + 1);
    file_data_req.offset = 0;
    file_data_req.size = file_size;
    
    int bytes_read = read_full(peer_fd, file_data, file_size);
    std::cout << "bytes read from peer : " << bytes_read << std::endl;

    // save file to storage
    fwrite_full(out_path.c_str(), file_data, file_size);

    // disconnect from peer
    disconnect_from_server(peer_fd);

    // disconnect from hub
    disconnect_from_server(hub_sockfd);

    return;
}


// server process 3 (listens for rfc requests)
void handle_rpc_server(struct dnode_details_struct *dnode_details) {

    
    std::cout << "RPC server :: Starting!!" << std::endl;
    char* rpc_buffer = (char *)malloc(2048); // 2MB all-purpose buffer
    int rpc_port = dnode_details->rpc_port;
    int sockfd = create_server(rpc_port);

    if(sockfd < 0) {
        std::cout << "RPC server :: Unable to create server with port " << rpc_port << std::endl;
        std::cout << "RPC server :: Terminating.. " << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "RPC server :: server created!!" << std::endl;

    if (listen(sockfd, 5) == 0) {
        std::cout << "RPC server :: listening for RPC commands on port " << rpc_port <<std::endl;
    }

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    while(1) {

        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("Accept error\n");
            std::exit(0);
        }

        std::cout << "RPC server :: new request" << std::endl;

        struct rpc_cmd_struct rpc_cmd;
        recv(newsockfd, &rpc_cmd, sizeof(rpc_cmd), 0);
        recv(newsockfd, rpc_buffer, rpc_cmd.payload_len + 1, 0);

        if (rpc_cmd.cmd_type == RPC_COMMAND_UPLOAD) {
            std::cout << "\nRPC upload command " << std::endl;
            char *file_path = rpc_buffer;
            handle_file_upload(dnode_details, newsockfd, file_path);

        } else if (rpc_cmd.cmd_type == RPC_COMMAND_DOWNLOAD) {
            std::cout << "\nRPC download command " << std::endl;
            char *file_name = rpc_buffer;
            handle_file_download(dnode_details, newsockfd, file_name);

        } else {
            std::cout << "Invalid RPC command " << std::endl;
        }
 
        close(newsockfd);
    }

    close(sockfd);
    std::exit(EXIT_SUCCESS);
}
