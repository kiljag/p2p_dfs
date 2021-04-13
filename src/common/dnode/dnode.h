
#ifndef _DNODE_H_
#define _DNODE_H_

#include <stdint.h>
#include <string>

using namespace std;

struct dnode_details {
    char* hub_ip;
    int hub_port;
    int hub_cmd_port;
    int dnode_data_port;
    int rpc_port;
    uint64_t uid;
    char* root_dir;
    char* files_dir;
};

#endif