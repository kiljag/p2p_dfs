
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

int create_server(int port) {

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
        printf("failed to bind to server port");
        std::exit(EXIT_FAILURE);
    }

    return sockfd;
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

