#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net.h"

using namespace std;

int create_server(short port) {

    int sockfd;
    struct sockaddr_in servaddr;

    // socket creation
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("socket creation failed..");
        std::exit(EXIT_FAILURE);
    }

    // assign ip, port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    //bind to port
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        // printf("failed to bind to server port %d");
        std::cout << "failed to bind to port " << port << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return sockfd;
}

int stop_server(int sockfd) {
    close(sockfd);
}

int connect_to_server(struct in_addr server_ip, short server_port) {

    // create a socket to contact hub
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Unable to create hub socket\n");
        exit(0);
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family	= AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    // serv_addr.sin_addr = server_port;
    serv_addr.sin_port = htons(server_port);
    
    // connect to hub
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to connect to hub!!");
        exit(0);
    }

    return sockfd;
}

int disconnect_from_server(int sockfd) {
    close(sockfd);
}



struct in_addr parse_ip_addr(char *ip_port) {
    
    std::string ip_port_str(ip_port);
    int del_pos = ip_port_str.find(":");
    std::string ip_addr_str = ip_port_str.substr(0, del_pos);

    struct in_addr ip_addr;
    inet_aton(ip_addr_str.c_str(), &ip_addr);
    
    return ip_addr;
}

short parse_port(char *ip_port) {

    std::string ip_port_str(ip_port);
    int del_pos = ip_port_str.find(":");
    std::string port_str = ip_port_str.substr(del_pos+1, ip_port_str.size());

    return (short)(std::stoi(port_str));
}

/* read from the fd until the buff contains {size} bytes*/
int read_full(int sockfd, void *buff, int size) {
    
    int offset = 0;
    while (offset < size) {
        int bytes_read = recv(sockfd, buff, size - offset, 0);
        if (bytes_read < 0) {
            return -1;
        }

        offset += bytes_read;
    }

    return offset;
}

/* write entire buff to fd */
int write_full(int sockfd, void *buff, int size) {

    int offset = 0;
    while (offset < size) {
        int bytes_written = send(sockfd, buff, size - offset, 0);
        if (bytes_written < 0) {
            return -1;
        }
        
        offset += bytes_written;
    }

    return offset;
}

int recv_full(int sockfd, void *buff, int size) {

    int offset = 0;
    while (offset < size) {
        int bytes_recv = recv(sockfd, buff, size - offset, 0);
        std::cout << "bytes recv : " << bytes_recv << std::endl;
        if (bytes_recv < 0) {
            perror("peer node failed\n");
            return -1;
        }

        offset += bytes_recv;
    }

    return offset;
}

int send_full(int sockfd, void *buff, int size) {

    int offset = 0;
    while (offset < size) {
        int bytes_sent = send(sockfd, buff, size - offset, 0);
        std::cout << "bytes sent : " << bytes_sent << std::endl;
        if (bytes_sent < 0) {
            perror("peer node failed\n");
            return -1;
        }
        
        offset += bytes_sent;
    }

    return offset;

}