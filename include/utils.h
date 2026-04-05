#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

typedef struct FileEntry {
    char filename[260];
    char hash[32];
    struct FileEntry *next;
} FileEntry;

/* Returns 1 if path exists, 0 otherwise. */
int path_exists(const char *path);

/* Creates a directory if it does not exist. */
int ensure_dir(const char *path);

/* Reads entire text file and returns heap buffer (caller frees). */
char *read_text_file(const char *path);

/* Writes text content to file, replacing existing content. */
int write_text_file(const char *path, const char *content);

/* Copies a file byte-for-byte from src to dst. */
int copy_file(const char *src, const char *dst);

/* Duplicates a C string on heap (caller frees). */
char *duplicate_string(const char *input);

/* Trims trailing newline and carriage return characters in-place. */
void trim_newline(char *text);

/* Appends a new file entry node to list. */
FileEntry *append_file_entry(FileEntry **head, const char *filename, const char *hash);

/* Finds a node by filename in the list. */
FileEntry *find_file_entry(FileEntry *head, const char *filename);

/* Frees the linked list. */
void free_file_list(FileEntry *head);

#endif