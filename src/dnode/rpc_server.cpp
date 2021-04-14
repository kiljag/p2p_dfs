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
            std::cout << "RPC upload command " << std::endl;

            char *in_file_path = rpc_buffer;
            int file_len = get_file_size(in_file_path);
            std::cout << "in_file_path : " << in_file_path << std::endl;
            std::cout << "file size : " << file_len << std::endl;

            string file_path_str(in_file_path);
            string file_name_str = file_path_str.substr(file_path_str.find_last_of("/") + 1, file_path_str.size());
            string out_path = string(dnode_details->files_dir) + string("/") + file_name_str;


            uint8_t *file_buff = (uint8_t *)malloc(file_len);
            size_t bytes_read = fread_full(in_file_path, file_buff, file_len);
            std::cout << "bytes read :: " << bytes_read << std::endl;

            // compute file hash
            struct fhash h = compute_hash(file_buff, file_len);
            printf("%016lx%016lx%016lx%016lx\n", h.a, h.b, h.c, h.d);
            uint64_t file_hash = reduce_hash(&h);
            printf("file hash : %016lx\n", file_hash);

            // send the file hash to the hub
            struct hub_cmd_struct hub_cmd;
            hub_cmd.cmd_type = FILE_UPLOAD;

            struct file_upload_req_struct file_upload_req;
            file_upload_req.data_node_uid = dnode_details->uid;
            file_upload_req.file_hash = file_hash;
            file_upload_req.file_index_data_len = 0;
            
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
            hub_addr.sin_port = htons(dnode_details->hub_port);
    
            // connect to hub
            if (connect(hub_sockfd, (struct sockaddr *)&hub_addr, sizeof(hub_addr)) < 0) {
                perror("Unable to connect to hub!!");
                exit(0);
            }

            std::cout << "uploading file metadata to hub.." << std::endl;
            send(hub_sockfd, &hub_cmd, sizeof(hub_cmd), 0);
            send(hub_sockfd, &file_upload_req, sizeof(file_upload_req), 0);

            close(hub_sockfd);

            // save the file in dnode files directory
            std::cout << "out file patth " << out_path << std::endl;
            size_t bytes_written = fwrite_full(out_path.c_str(), file_buff, file_len);
            std::cout << "bytes written  :: " << bytes_written << std::endl;
            


        } else if (rpc_cmd.cmd_type == RPC_COMMAND_DOWNLOAD) {
            std::cout << "RPC download command " << std::endl;

        } else {
            std::cout << "Invalid RPC command " << std::endl;
        }

        
        close(newsockfd);
        break;

    }

    close(sockfd);
    std::exit(EXIT_SUCCESS);
}
