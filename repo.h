#ifndef REPO_H
#define REPO_H

#include "utils.h"

/* Initializes .mygit repository structure. */
int repo_init(void);

/* Returns 1 if repository exists, 0 otherwise. */
int repo_exists(void);

/* Adds a file to object storage and staging index. */
int repo_add_file(const char *filename);

/* Loads staging index entries into linked list. */
int repo_read_index(FileEntry **out_head);

/* Persists linked list entries to staging index. */
int repo_write_index(FileEntry *head);

/* Reads HEAD commit id into output buffer. */
int repo_read_head(char *out_commit, size_t out_size);

/* Updates HEAD commit id. */
int repo_write_head(const char *commit_id);

#endif