
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

#include "hub/hub.h"
#include "hub/dnode_req_server.h"
#include "hub/hub_req_server.h"

#include "util/net.h"
#include "util/hash.h"


using namespace std;

// filehash, list_of_dnode_uids mapping
map<uint64_t, vector<uint64_t>> fhash_map;

// to store file_meta_details (file_hash, file_metdata)
map<uint64_t, struct file_metadata_struct *> file_metadata_map; 

// to store file_index_data (file_hash, file_indexdata)
map<uint64_t, uint8_t *> file_indexdata_map;

// filename, filehash mapping
map<string, uint64_t> fname_map;

// to store dnode_details
map<uint64_t, struct dnode_struct *> dnodes_map;


// peer hub details
map<uint64_t, struct peer_hub_struct *> peer_hubs_map;

struct hub_details_struct hub_details;


void initialize_hub() {

}

/*
./hub root_dir hub_cmd_port dnode_cmd_port
*/

int main(int argc, char** argv) {

    if (argc < 4) {
        std::cout << "Insufficient arguments" << std::endl;
        std::exit(0);
    }

    char* hub_root_dir = argv[1];
    hub_details.hub_cmd_port = std::stoi(argv[2]);
    hub_details.dnode_cmd_port = std::stoi(argv[3]);
    memcpy(hub_details.hub_root_dir, hub_root_dir, strlen(hub_root_dir) + 1);

    printf("hub root directory : %s\n", hub_details.hub_root_dir);
    printf("hub_cmd_port (listen for peer hubs) : %d\n", hub_details.hub_cmd_port);
    printf("hub_dnode_port (listenf for dnodes) : %d\n", hub_details.dnode_cmd_port);


    pthread_t hub_req_server_tid;
    if (pthread_create(&hub_req_server_tid, NULL, handle_hub_req_server, NULL) != 0) {
        perror("Failed to create hub_req_server thread");
        exit(EXIT_FAILURE);
    }

    pthread_t dnode_req_server_tid;
    if (pthread_create(&dnode_req_server_tid, NULL, handle_dnode_req_server, NULL) != 0) {
        perror("Failed to create dnode_req_server thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(hub_req_server_tid, NULL);
    pthread_join(dnode_req_server_tid, NULL);
    

    // pid_t dnode_req_server_pid = fork();
    // if (dnode_req_server_pid == 0) {
    //     handle_dnode_req_server();
    // }
    // cout << "data req server pid : " << dnode_req_server_pid << endl;

    // int wstatus;
    // pid_t child_pid = wait(&wstatus);
    // std::cout << "child pid : " << child_pid << endl;
    
    return 0;
}