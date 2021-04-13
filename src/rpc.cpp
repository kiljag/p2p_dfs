#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

using namespace std;

/*
cmd :: ./rfc hostip:port -u file_path
cmd :: ./rfc hostip:port -d file_name
*/

#define COMMAND_UPLOAD 0x01
#define COMMAND_DOWNLOAD 0x02
#define COMMAND_KILL 0x03

int main(int argc, char* argv[]) {

    if (argc < 4) {
        printf("insufficient arguments..");
        exit(EXIT_FAILURE);
    }

    string host_ip_port(argv[1]);
    int del_pos = host_ip_port.find(":");
    string dnode_ip = host_ip_port.substr(0, del_pos);
    int dnode_port = stoi(host_ip_port.substr(del_pos+1, host_ip_port.size()));

    cout << "dnode ip : " << dnode_ip << endl;
    cout << "dnode port : " << dnode_port << endl;
    cout << "command : " << argv[2] << endl;
    cout << "file_path : " << argv[3] << endl;
    

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("failed to create socket\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    inet_aton(dnode_ip.c_str(), &serv_addr.sin_addr);
    serv_addr.sin_port = htons((short)dnode_port);

    
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("unable to connect to server..\n");
        exit(0);
    }

    char buffer[500];
    if (string(argv[2]) == "-u") {
        buffer[0] = COMMAND_UPLOAD;
    } else if (string(argv[2]) == "-d") {
        buffer[0] = COMMAND_DOWNLOAD;
    } else {
        printf("Invalid command, %s\n", argv[2]);
        exit(0);
    }
    int payload_len = strlen(argv[3]);
    memcpy(buffer + 1, &payload_len, 4);
    memcpy(buffer + 5, argv[3], payload_len);

    send(sockfd, buffer, payload_len + 5, 0);
    return 0;
    
}