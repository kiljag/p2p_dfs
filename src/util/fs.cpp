
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "fs.h"


size_t get_file_size(char *file_path) {
    struct stat st;
    stat(file_path, &st);
    return st.st_size;
}

/*to do : error checking*/
int fread_full(int fd, uint8_t *buff, size_t size) {

    // int fd = open(file_path, O_RDONLY);
    // if (fd < 0) {
    //     perror("unable to open file (read only mode).\n");
    //     return -1;
    // }

    int offset = 0;
    while(offset < size) {
        int bytes_read = read(fd, buff + offset, size - offset);
        if (bytes_read < 0) {
            perror("error in reading file\n");
            return -1;
        }
        offset += bytes_read;
    }

    return offset;
}

/*to do : error checking*/
int fwrite_full(int fd, uint8_t *buff, size_t size) {

    // int fd = open(file_path, O_CREAT | O_WRONLY, 0644);
    // if (fd < 0) {
    //     perror("unable to open file (write only mode).\n");
    //     return -1;
    // }

    int offset = 0;
    while(offset < size) {
        int bytes_written  = write(fd, buff + offset, size - offset);
        if (bytes_written < 0) {
            perror("error in writing file\n");
            return -1;
        }
        offset += bytes_written;
    }

    return offset;
}