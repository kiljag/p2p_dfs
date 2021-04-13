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

#include "../util/hash.h"
#include "../util/net.h"


// server process 3 (listens for rfc requests)
void handle_rpc_server(struct dnode_details *node_details) {


    std::cout << "RPC server :: Starting!!" << std::endl;
    char* rfc_buffer = (char *)malloc(2048); // 2MB all-purpose buffer
    int rpc_port = node_details->rpc_port;
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

        std::cout << "RPC server :: Handling RPC request" << std::endl;
        recv(newsockfd, rfc_buffer, 2048, 0);

        if(rfc_buffer[0] == RPC_COMMAND_UPLOAD) {
            printf("command upload\n");
        } else if (rfc_buffer[0] == RPC_COMMAND_DOWNLOAD) {
            printf("command download\n");
        }

        int file_path_len;
        memcpy(&file_path_len, rfc_buffer+1, 4);
        std::cout << "payload len" << file_path_len << endl;

        string file_path(rfc_buffer + 5, rfc_buffer + 5 + file_path_len);
        string file_name = file_path.substr(file_path.find_last_of("/") + 1, file_path.size());

        std::cout << "file path : " << file_path << endl;
        std::cout << "file name : " << file_name << endl;

        if (rfc_buffer[0] == RPC_COMMAND_UPLOAD) {
            // finding hash of the file.

            // int read_fd = open(file_path.c_str(), O_RDONLY);
            // if (read_fd < 0) {
            //     perror("unable to open the input file path!!\n");
            //     break;
            // }

            // int offset = 0;
            // while(read())
            


            FILE *f  = fopen(file_path.c_str(), "rb");
            if (!f) {
                perror("cant open file");
                break;
            }

            fseek(f, 0L, SEEK_END);
            size_t fsz = ftell(f);
            rewind(f);

            uint8_t *d = (uint8_t *)malloc(fsz + 1);
            fread(d, 1, fsz, f);

            struct fhash h = compute_hash(d, fsz);
            printf("%016lx%016lx%016lx%016lx\n", h.a, h.b, h.c, h.d);
            uint64_t file_hash = reduce_hash(&h);
            printf("file hash : %016lx\n", file_hash);

            
            string out_path = string(node_details->files_dir) + string("/") + file_name;
            std::cout << "out_path : " << out_path << endl;
            FILE *fw  = fopen(out_path.c_str(), "wb");
            fwrite(d, 1, fsz+1, fw);
            // ifstream infile;
            // infile.open(file_path, ios::binary | ios::in);


            

        }

        close(newsockfd);
        break;
    }

    std::exit(EXIT_SUCCESS);
}
