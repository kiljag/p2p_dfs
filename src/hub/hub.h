
#ifndef _HUB_H_
#define _HUB_H_

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FILE_CHUNK_SIZE (1024*1024)  // 1 mb
#define FILE_CHUNK_HASH_SIZE 8 // 64 bits

// commands supported by a hub
#define NODE_JOIN 0x0001
#define NODE_HELLO 0x0002
#define FILE_UPLOAD 0x0003
#define FILE_DOWNLOAD 0x0004
#define FILE_DOWNLOADED_ACK 0x0005

struct hub_cmd_struct {
    uint16_t cmd_type;
};

// hub only data structures

struct dnode_struct{ // details about a dnode
    uint64_t uid;
    struct in_addr ip;
    short port;
    uint16_t flags;
};

struct file_metadata_struct { // details about a file
    uint64_t file_hash;
    int file_size;
    int num_chunks;
    int file_index_data_size;
};

// public data structures (hub and peer dnodes)

struct peer_dnode_struct {
    struct in_addr ip;
    short port;
    short flags;
};

// NEW_NODE_JOIN

struct node_join_req_struct {
    struct in_addr ip;
    short port;
    uint16_t flags;
};

struct node_join_res_struct {
    uint64_t uid;
};

// NODE HELLO
struct node_hello_req_struct {
    uint64_t dnode_id;
    struct in_addr ip;
    short port;
    uint16_t flags;
};

struct node_hello_res_struct {
    uint64_t uid;
};

// FILE UPLOAD 

struct file_upload_req_struct {
    uint64_t dnode_uid;
    uint64_t file_hash;
    int file_size;
    char file_name[64]; // assuming all the files of size less than this.
    int num_chunks;
    int file_index_data_size;
};

struct file_upload_res_struct {

};

// FILE DOWNLOAD

struct file_download_req_struct {
    uint64_t dnode_uid;
    char file_name[64];
};

struct file_download_res_struct {
    uint64_t file_hash;
    int file_size;
    int num_chunks;
    int num_peer_dnodes;
    int file_index_data_size;
};

// FILE DOWNLOADED ACK 

struct file_downloaded_ack_struct {
    uint64_t dnode_uid;
    uint64_t file_hash;
};


#endif