
#ifndef _HUB_H_
#define _HUB_H_

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// commands supported by a hub
#define NODE_JOIN 0x0001
#define NODE_HELLO 0x0002
#define FILE_UPLOAD 0x0003
#define FILE_DOWNLOAD 0x0004


// hub only data structures

struct dnode_struct{
    uint64_t uid;
    struct in_addr ip;
    int port;
    int flags;
};


// public data structures

struct dnode_compact_struct {
    struct in_addr ip;
    int port;
    int flags;
};

struct node_join_struct {
    uint64_t uid;
    struct in_addr ip;
    int port;
    int flags;
};

struct file_upload_struct {
    uint64_t data_node_uid;
    uint64_t file_hash;
};

struct file_download_struct {
    uint64_t data_node_uid;
    uint64_t file_hash;
};

struct file_download_response_struct {
    int numdnodes;
    dnode_compact_struct *dnodes;
};

#endif