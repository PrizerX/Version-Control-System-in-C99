#ifndef COMMIT_H
#define COMMIT_H

#include "stack.h"
#include "utils.h"

typedef struct Commit {
    char id[32];
    char parent[32];
    char message[256];
    long timestamp;
    FileEntry *files;
} Commit;

/* Creates a commit from current index with message. */
int create_commit(const char *message, Stack *stack, char *out_commit_id, size_t out_size);

/* Prints commit history from HEAD to root. */
int print_commit_log(void);

/* Loads a commit file into Commit structure. */
int load_commit(const char *commit_id, Commit *out_commit);

/* Frees heap resources stored inside Commit structure. */
void free_commit(Commit *commit);

/* Restores working files from a specific commit id. */
int checkout_commit(const char *commit_id);

/* Builds stack state from current HEAD commit chain. */
int build_commit_stack(Stack *stack);

/* Undoes latest commit and restores previous commit state. */
int undo_last_commit(Stack *stack);

#endif