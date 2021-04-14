
#ifndef _DNODE_H_
#define _DNODE_H_

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RPC_REQ_UPLOAD 0x0001
#define RPC_REQ_DOWNLOAD 0x0002
#define RPC_REQ_KILL 0x0003

#define RPC_RES_SUCCEES 0x0101
#define RPC_RES_FAILURE 0x0102

#define DATA_TRANSFER_SUCCESS 0x00010001
#define DATA_TRANSFER_FAILURE 0X00010001

// RPC rerquest and response

struct rpc_req_struct {
    uint16_t req_type;
    short payload_len;
};

struct rpc_res_struct {
    uint16_t res_type;
    short payload_len;
};

// Data server, File request
struct file_data_req_struct {
    uint64_t file_hash;
    char file_name[64];
    int offset;
    int size;
};

struct file_data_res_struct {
    uint64_t res_type;
    int payload_len;
};

struct dnode_details_struct {
    struct in_addr hub_ip;
    short hub_port;
    struct in_addr dnode_ip;
    short hub_cmd_port;
    short dnode_data_port;
    short rpc_port;
    uint64_t uid;
    char root_dir[256];
    char files_dir[256];
    char meta_dir[256];
    char uid_file_path[256];
};

#endif