
#ifndef _DNODE_H_
#define _DNODE_H_

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RPC_COMMAND_UPLOAD 0x01
#define RPC_COMMAND_DOWNLOAD 0x02
#define RPC_COMMAND_KILL 0x03


struct dnode_details {
    struct in_addr hub_ip;
    int hub_port;
    struct in_addr dnode_ip;
    int hub_cmd_port;
    int dnode_data_port;
    int rpc_port;
    uint64_t uid;
    char* root_dir;
    char* files_dir;
};

#endif