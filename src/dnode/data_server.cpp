#include <iostream>
#include <fstream>
#include <string>

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
#include "data_server.h"

#include "../hub/hub.h"
#include "../util/hash.h"
#include "../util/net.h"
#include "../util/fs.h"

using namespace std;

void handle_data_server(struct dnode_details_struct *dnode_details) {

    std::cout << "Data server :: starting !!" << std::endl;
    uint8_t *data_buffer = (uint8_t *)malloc(2048);
    int data_port = dnode_details->dnode_data_port;
    int sockfd = create_server(data_port);

    if(sockfd < 0) {
        std::cout << "Data server :: Unable to create server with port " << data_port << std::endl;
        std::cout << "Data server :: Terminating.. " << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Data server :: server created!!" << std::endl;

    if (listen(sockfd, 5) == 0) {
        std::cout << "Data server :: listening for data requests on port " << data_port << std::endl;
    }

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    while(1) {
        
        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("Accept error\n");
            std::exit(0);
        }

        struct file_data_req_struct file_data_req;
        read_full(newsockfd, &file_data_req, sizeof(file_data_req));

        string files_dir = string(dnode_details->files_dir); 
        string file_path = files_dir + string("/") + string(file_data_req.file_name);

        /*for now assume, the request is for entire file*/
        int file_size = file_data_req.size - file_data_req.offset;
        uint8_t *file_data = (uint8_t *)malloc(file_size);
        fread_full(file_path.c_str(), file_data, file_size);

        write_full(newsockfd, file_data, file_size);

        close(newsockfd);
    }

    close(sockfd);
}