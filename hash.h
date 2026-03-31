#ifndef HASH_H
#define HASH_H

#include <stddef.h>

/* Computes a simple deterministic hash of a null-terminated string. */
unsigned long hash_string(const char *input);

/* Computes a hash for an arbitrary byte buffer and writes hex text to out_hash. */
void hash_buffer_hex(const unsigned char *data, size_t length, char *out_hash, size_t out_size);

#endif