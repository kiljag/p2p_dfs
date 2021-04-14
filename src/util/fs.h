
#ifndef _FS_H_
#define _FS_h_

#include <sys/types.h>
#include <stdint.h>

/* reads the entire file */
size_t get_file_size(char *file_path);

size_t fread_full(const char *file_path, uint8_t *buf, size_t size);

size_t fwrite_full(const char *file_path, uint8_t *buf, size_t size);

#endif