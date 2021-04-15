
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "hub.h"
#include "hub_req_server.h"
#include "../util/net.h"
#include "../util/hash.h"


using namespace std;

// filehash, list_of_dnode_uids mapping
extern map<uint64_t, vector<uint64_t>> fhash_map;

// to store file_meta_details (file_hash, file_metdata)
extern map<uint64_t, struct file_metadata_struct *> file_metadata_map; 

// to store file_index_data (file_hash, file_indexdata)
extern map<uint64_t, uint8_t *> file_indexdata_map;

// filename, filehash mapping
extern map<string, uint64_t> fname_map;

// to store dnode_details
extern map<uint64_t, struct dnode_struct *> dnodes_map;

extern hub_details_struct hub_details;

void *handle_hub_req_server(void *args) {

    // sleep(10);
    // exit(0);
} 