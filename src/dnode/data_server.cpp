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
        
        int peer_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (peer_sockfd < 0) {
            perror("Accept error\n");
            std::exit(0);
        }

        std::cout << "\nData server :: Handling file data request " << std::endl;

        // get file data request
        struct file_data_req_struct file_data_req;
        recv_full(peer_sockfd, &file_data_req, sizeof(file_data_req));
        
        // parse file data response
        int offset = file_data_req.offset;
        int size = file_data_req.size;
        string files_dir = string(dnode_details->files_dir); 
        string file_path = files_dir + string("/") + string(file_data_req.file_name);

        // fill response
        struct file_data_res_struct file_data_res;

        int file_size = get_file_size(file_path.c_str());
        if (offset < 0 || offset + size > file_size) { // can not server data request
            file_data_res.res_type = DATA_TRANSFER_FAILURE;
            file_data_res.payload_len =  0;    
            send_full(peer_sockfd, &file_data_res, sizeof(file_data_res));
            close(peer_sockfd);
            continue;
        }

        file_data_res.res_type = DATA_TRANSFER_SUCCESS;
        file_data_res.payload_len = size;

        int read_fd = open(file_path.c_str(), O_RDONLY);
        lseek(read_fd, offset, SEEK_SET);
        
        uint8_t *chunk_data = (uint8_t *)malloc(sizeof(size));
        int bytes_read = fread_full(read_fd, chunk_data, size);
        int bytes_sent = send_full(peer_sockfd, chunk_data, size);

        std::cout << "bytes read : " << bytes_read << std::endl;
        std::cout << "bytes sent : " << bytes_sent << std::endl;

        free(chunk_data);
        close(read_fd);
        close(peer_sockfd);
    }

    close(sockfd);
}
