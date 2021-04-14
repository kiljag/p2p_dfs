
#ifndef _NETCALLS_H_
#define _NETCALLS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

int create_server(int port);

/* input is string ipaddr:port */
struct in_addr parse_ip_addr(char *ip_port);
short parse_port(char *ip_port);


#endif