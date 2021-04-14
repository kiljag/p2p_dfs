
#ifndef _NETCALLS_H_
#define _NETCALLS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

/* utility functions to create and stop a server*/
int create_server(short port);
int stop_server(int sockfd);

/* functions to connect to and disconnect from a server */
int connect_to_server(struct in_addr server_ip, short server_port);
int disconnect_from_server(int sockfd);

/* input is string ipaddr:port */
struct in_addr parse_ip_addr(char *ip_port);
short parse_port(char *ip_port);

/* some network read utilities */
int read_full(int sockfd, void *buff, int size);
int write_full(int sockfd, void *buff, int size); 

int recv_full(int sockfd, void *buff, int size);
int send_full(int sockfd, void *buff, int size);

#endif