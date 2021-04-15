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

extern struct dnode_details_struct dnode_details;

void handle_file_upload(int rpc_cli_fd, char *file_path) {

    uint8_t chunk_buffer[FILE_CHUNK_SIZE];

    // get file size
    int file_size = get_file_size(file_path);
    std::cout << "in_file_path : " << file_path << std::endl;
    std::cout << "file size : " << file_size << std::endl;

    string file_path_str(file_path);
    string file_name_str = file_path_str.substr(file_path_str.find_last_of("/") + 1, file_path_str.size());
    string out_path = string(dnode_details.files_dir) + string("/") + file_name_str;
    std::cout << "out_file_path : " << out_path << std::endl; 

    int num_complete_chunks = file_size / FILE_CHUNK_SIZE;
    int num_chunks = num_complete_chunks;
    int partial_chunk_size = file_size % FILE_CHUNK_SIZE;
    if (partial_chunk_size > 0) {
        num_chunks += 1;
    }
    std::cout << "chunk size : " << FILE_CHUNK_SIZE << std::endl;
    std::cout << "num chunks : " << num_chunks << std::endl;
    std::cout << "partial chunk size : " << partial_chunk_size << std::endl;

    int file_index_data_size = num_chunks * FILE_CHUNK_HASH_SIZE;
    uint8_t *file_index_data = (uint8_t *)malloc(file_index_data_size);
    uint64_t *chunk_hash_list = (uint64_t *)file_index_data;

    // open read and write fds.
    int read_fd = open(file_path, O_RDONLY);
    int write_fd = open(out_path.c_str(), O_CREAT | O_WRONLY, 0644);

    int bytes_read = 0;
    int bytes_written = 0;
    int total_bytes_read = 0;
    int total_bytes_written = 0;
    uint64_t chunk_hash;

    // read, hash, write each complete chunk
    for (int i = 0; i < num_complete_chunks; i++) {
        
        bytes_read = fread_full(read_fd, chunk_buffer, FILE_CHUNK_SIZE);
        total_bytes_read += bytes_read;

        chunk_hash = compute_hash(chunk_buffer, FILE_CHUNK_SIZE);
        chunk_hash_list[i] = chunk_hash;

        bytes_written = fwrite_full(write_fd, chunk_buffer, FILE_CHUNK_SIZE);
        total_bytes_written += bytes_written;

        printf("i : %02d, chunk_hash : %08lx\n", i + 1, chunk_hash);
    }

    if (partial_chunk_size > 0) {
        // above operations for last partial chunk
        bytes_read = fread_full(read_fd, chunk_buffer, partial_chunk_size);
        total_bytes_read += bytes_read;

        chunk_hash = compute_hash(chunk_buffer, partial_chunk_size);
        chunk_hash_list[num_chunks - 1] = chunk_hash;

        bytes_written = fwrite_full(write_fd, chunk_buffer, partial_chunk_size);
        total_bytes_written += bytes_written;

        printf("i : %02d, chunk_hash : %08lx\n", num_chunks, chunk_hash);
    }
    

    std::cout << "total bytes read : " << total_bytes_read << std::endl;
    std::cout << "total bytes written : " << total_bytes_written << std::endl;
    
    close(read_fd);
    close(write_fd);

    // compute file hash
    uint64_t file_hash = compute_hash(file_index_data, file_index_data_size);
    printf("file hash : %08lx\n", file_hash);

    // connect to hub
    int hub_sockfd = connect_to_server(dnode_details.hub_ip, dnode_details.hub_port);

    // send hub request (file upload request)
    struct hub_cmd_struct hub_cmd;
    hub_cmd.cmd_type = FILE_UPLOAD;
    send_full(hub_sockfd, &hub_cmd, sizeof(hub_cmd));

    // send the file upload request details
    struct file_upload_req_struct file_upload_req;
    file_upload_req.dnode_uid = dnode_details.uid;
    file_upload_req.file_hash = file_hash;
    file_upload_req.file_size = file_size;
    memcpy(file_upload_req.file_name, file_name_str.c_str(), file_name_str.size() + 1); // include null byte
    file_upload_req.num_chunks = num_chunks;
    file_upload_req.file_index_data_size = file_index_data_size;
    send_full(hub_sockfd, &file_upload_req, sizeof(file_upload_req));

    // send the index data
    send_full(hub_sockfd, file_index_data, file_index_data_size);

    /* TODO : recive response from server */

    // disconnect from hub
    disconnect_from_server(hub_sockfd);

    // send success response to rpc client
    struct rpc_res_struct rpc_res;
    rpc_res.res_type = RPC_RES_SUCCEES;
    rpc_res.payload_len = 0;
    send_full(rpc_cli_fd, &rpc_res, sizeof(rpc_res));

    return;
}


void multi_threaded_download(struct file_download_res_struct *file_download_res,
                            struct peer_dnode_struct * peer_dnodes_list,
                            uint64_t *chunk_hashes, char *file_name) {
    
    

    return;
}

void naive_download(struct file_download_res_struct *file_download_res,
                    struct peer_dnode_struct * peer_dnodes_list,
                    uint64_t *chunk_hashes, char *file_name) {
    
    // retrieve last peer details
    int num_peer_dnodes = file_download_res->num_peer_dnodes;
    struct in_addr peer_ip = peer_dnodes_list[num_peer_dnodes - 1].ip;
    short peer_port = peer_dnodes_list[num_peer_dnodes - 1].port;

    int file_hash = file_download_res->file_hash;
    int file_size = file_download_res->file_size;
    uint8_t* file_data = (uint8_t *)malloc(file_size);
    std::cout << "file size : " << file_size << endl;

    // connect to peer
    std::cout << "connecting to peer dnode.." << std::endl;
    std::cout << "peer port " << peer_port << std::endl;
    int peer_sockfd = connect_to_server(peer_ip, peer_port);

    // fill data request details (download entire data)
    file_data_req_struct file_data_req;
    file_data_req.file_hash = file_hash;
    memcpy(file_data_req.file_name, file_name, strlen(file_name) + 1);
    file_data_req.offset = 0;
    file_data_req.size = file_size;

    // send data request details to peer
    send_full(peer_sockfd, &file_data_req, sizeof(file_data_req));

    // recv chunk data from server
    int bytes_recv= recv_full(peer_sockfd, file_data, file_size);
    std::cout << "bytes recv from peer : " << bytes_recv << std::endl;

    // save file to storage
    string file_name_str(file_name);
    string out_path = string(dnode_details.files_dir) + string("/") + file_name_str;
    int write_fd = open(out_path.c_str(), O_CREAT | O_WRONLY, 0644);
    int bytes_written  = fwrite_full(write_fd, file_data, file_size);
    std::cout << "bytes written (fs) : " << bytes_written << std::endl;
    close(write_fd);

    free(file_data);

    //disconnect from peer
    disconnect_from_server(peer_sockfd);
    return;
}

void handle_file_download(int rpc_cli_fd, char *file_name) {

    // connect to hub
    int hub_sockfd = connect_to_server(dnode_details.hub_ip, dnode_details.hub_port);

    // send download hub command
    struct hub_cmd_struct hub_cmd;
    hub_cmd.cmd_type = FILE_DOWNLOAD;
    send_full(hub_sockfd, &hub_cmd, sizeof(hub_cmd));

    // send download request details
    struct file_download_req_struct file_download_req;
    file_download_req.dnode_uid = dnode_details.uid;
    memcpy(file_download_req.file_name, file_name, strlen(file_name) + 1);
    send_full(hub_sockfd, &file_download_req, sizeof(file_download_req));

    // recv file download response from server
    struct file_download_res_struct file_download_res;
    recv(hub_sockfd, &file_download_res, sizeof(file_download_res), 0);
    int num_peer_dnodes = file_download_res.num_peer_dnodes;
    int file_index_data_size = file_download_res.file_index_data_size;

    printf("file_hash : %016lx\n", file_download_res.file_hash);
    printf("file size : %d \n", file_download_res.file_size);
    printf("num_chunks: %d \n", file_download_res.num_chunks);
    printf("num peer nodes:  %d\n", num_peer_dnodes);

    // intialize file index data and peer dnodes data
    uint8_t *file_index_data = (uint8_t *)malloc(file_index_data_size);
    int peer_dnodes_list_size = sizeof(struct peer_dnode_struct) * num_peer_dnodes;
    struct peer_dnode_struct * peer_dnodes_list;
    peer_dnodes_list = (struct peer_dnode_struct *)malloc(peer_dnodes_list_size);
    
    // recieve file index data
    recv_full(hub_sockfd, file_index_data, file_index_data_size);
    uint64_t* chunk_hashes = (uint64_t *)file_index_data;

    // recv peer nodes list
    read_full(hub_sockfd, peer_dnodes_list,  peer_dnodes_list_size);
    
    // disconnect from hub
    disconnect_from_server(hub_sockfd);


    /*
    For now, download the entire data from single peer node
    */
    naive_download(&file_download_res, peer_dnodes_list, chunk_hashes, file_name);

    // connect to hub
    hub_sockfd = connect_to_server(dnode_details.hub_ip, dnode_details.hub_port);
    
    // send file_downloaded_ack request to hub
    // struct hub_cmd_struct hub_cmd;
    // hub_cmd.cmd_type = FILE_DOWNLOADED_ACK;
    // send_full(hub_sockfd, &hub_cmd, )
    send_cmd_to_hub(hub_sockfd, FILE_DOWNLOADED_ACK);

    // send ack to hub indicating this node is a source of file
    struct file_downloaded_ack_struct file_downloaded_ack;
    file_downloaded_ack.dnode_uid = dnode_details.uid;
    file_downloaded_ack.file_hash = file_download_res.file_hash;
    send_full(hub_sockfd, &file_downloaded_ack, sizeof(file_downloaded_ack));

    // disconnect from server
    disconnect_from_server(hub_sockfd);

    // send success response to rpc client
    struct rpc_res_struct rpc_res;
    rpc_res.res_type = RPC_RES_SUCCEES;
    rpc_res.payload_len = 0;
    send_full(rpc_cli_fd, &rpc_res, sizeof(rpc_res));

    return;
}


// server process 3 (listens for rfc requests)
void handle_rpc_server() {

    std::cout << "RPC server :: Starting!!" << std::endl;
    char* rpc_buffer = (char *)malloc(2048); // 2MB all-purpose buffer
    int rpc_port = dnode_details.rpc_port;
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

        std::cout << "\nRPC server :: Handling request" << std::endl;

        struct rpc_req_struct rpc_req;
        recv_full(newsockfd, &rpc_req, sizeof(rpc_req));
        recv_full(newsockfd, rpc_buffer, rpc_req.payload_len);

        if (rpc_req.req_type == RPC_REQ_UPLOAD) {
            std::cout << "RPC server :: upload request " << std::endl;
            char *file_path = rpc_buffer;
            handle_file_upload(newsockfd, file_path);

        } else if (rpc_req.req_type == RPC_REQ_DOWNLOAD) {
            std::cout << "RPC server :: download request " << std::endl;
            char *file_name = rpc_buffer;
            handle_file_download(newsockfd, file_name);

        } else {
            std::cout << "Invalid RPC command " << std::endl;
        }
 
        close(newsockfd);
    }

    close(sockfd);
    std::exit(EXIT_SUCCESS);
}
