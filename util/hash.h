
#ifndef _HASH_H_
#define _HASH_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct hash {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    uint64_t d;
};

struct hash compute_hash(void * data, size_t l);

uint64_t reduce_hash(struct hash *hash);

int compare_hashes(struct hash *hash1, struct hash);

#endif