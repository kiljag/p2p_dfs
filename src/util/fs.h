
#ifndef _FS_H_
#define _FS_h_

#include <sys/types.h>
#include <stdint.h>

/* reads the entire file */
size_t get_file_size(char *file_path);

int fread_full(int fd, uint8_t *buf, size_t size);

int fwrite_full(int fd, uint8_t *buf, size_t size);



#endif