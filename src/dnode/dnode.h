
#ifndef _DNODE_H_
#define _DNODE_H_

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RPC_COMMAND_UPLOAD 0x0001
#define RPC_COMMAND_DOWNLOAD 0x0002
#define RPC_COMMAND_KILL 0x0003

struct rpc_cmd_struct {
    uint16_t cmd_type;
    short payload_len;
};

struct file_data_req_struct {
    uint64_t file_hash;
    char file_name[64];
    int offset;
    int size;
};

struct dnode_details_struct {
    struct in_addr hub_ip;
    short hub_port;
    struct in_addr dnode_ip;
    short hub_cmd_port;
    short dnode_data_port;
    short rpc_port;
    uint64_t uid;
    char root_dir[128];
    char files_dir[128];
    char meta_dir[128];
};

#endif