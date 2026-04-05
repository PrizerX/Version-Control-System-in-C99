#include "hash.h"

#include <stdio.h>

unsigned long hash_string(const char *input) {
    unsigned long hash = 5381UL;
    int c;

    while ((c = *input++) != 0) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }
    return hash;
}

void hash_buffer_hex(const unsigned char *data, size_t length, char *out_hash, size_t out_size) {
    size_t i;
    unsigned long hash = 5381UL;

    for (i = 0; i < length; ++i) {
        hash = ((hash << 5) + hash) + (unsigned long)data[i];
    }

    if (out_size > 0) {
        (void)snprintf(out_hash, out_size, "%08lx", hash);
    }
}