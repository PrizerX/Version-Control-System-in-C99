#include "commit.h"

#include "hash.h"
#include "repo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COMMITS_DIR ".mygit/commits"
#define OBJECTS_DIR ".mygit/objects"

static int write_commit_file(const Commit *commit) {
    char path[512];
    FILE *fp;
    FileEntry *entry;

    (void)snprintf(path, sizeof(path), "%s/%s.txt", COMMITS_DIR, commit->id);
    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "commit_id:%s\n", commit->id);
    fprintf(fp, "parent:%s\n", commit->parent);
    fprintf(fp, "timestamp:%ld\n", commit->timestamp);
    fprintf(fp, "message:%s\n", commit->message);
    fprintf(fp, "files:\n");

    entry = commit->files;
    while (entry != NULL) {
        fprintf(fp, "%s|%s\n", entry->filename, entry->hash);
        entry = entry->next;
    }

    fclose(fp);
    return 0;
}

int create_commit(const char *message, char *out_commit_id, size_t out_size) {
    FileEntry *index_head = NULL;
    FileEntry *entry;
    char parent[32];
    char input_buffer[4096] = {0};
    Commit commit;

    if (!repo_exists()) {
        fprintf(stderr, "Not a mygit repository. Run 'mygit init' first.\n");
        return -1;
    }

    if (message == NULL || strlen(message) == 0) {
        fprintf(stderr, "Commit message must not be empty.\n");
        return -1;
    }

    if (repo_read_index(&index_head) != 0 || index_head == NULL) {
        fprintf(stderr, "Nothing to commit. Use 'mygit add <file>' first.\n");
        return -1;
    }

    if (repo_read_head(parent, sizeof(parent)) != 0 || strcmp(parent, "none") == 0) {
        (void)snprintf(parent, sizeof(parent), "none");
    }

    (void)snprintf(input_buffer, sizeof(input_buffer), "%s|%s|%ld", message, parent, (long)time(NULL));
    entry = index_head;
    while (entry != NULL) {
        (void)strncat(input_buffer, "|", sizeof(input_buffer) - strlen(input_buffer) - 1U);
        (void)strncat(input_buffer, entry->filename, sizeof(input_buffer) - strlen(input_buffer) - 1U);
        (void)strncat(input_buffer, ":", sizeof(input_buffer) - strlen(input_buffer) - 1U);
        (void)strncat(input_buffer, entry->hash, sizeof(input_buffer) - strlen(input_buffer) - 1U);
        entry = entry->next;
    }

    (void)snprintf(commit.id, sizeof(commit.id), "%08lx", hash_string(input_buffer));
    (void)snprintf(commit.parent, sizeof(commit.parent), "%s", parent);
    (void)snprintf(commit.message, sizeof(commit.message), "%s", message);
    commit.timestamp = (long)time(NULL);
    commit.files = index_head;

    if (write_commit_file(&commit) != 0) {
        fprintf(stderr, "Failed to write commit file.\n");
        free_file_list(index_head);
        return -1;
    }

    if (repo_write_head(commit.id) != 0) {
        fprintf(stderr, "Failed to update HEAD.\n");
        free_file_list(index_head);
        return -1;
    }

    if (out_commit_id != NULL && out_size > 0) {
        (void)snprintf(out_commit_id, out_size, "%s", commit.id);
    }

    printf("Committed as %s\n", commit.id);
    free_file_list(index_head);
    return 0;
}

int load_commit(const char *commit_id, Commit *out_commit) {
    char path[512];
    char *content;
    char *line;

    if (commit_id == NULL || out_commit == NULL) {
        return -1;
    }

    (void)snprintf(path, sizeof(path), "%s/%s.txt", COMMITS_DIR, commit_id);
    content = read_text_file(path);
    if (content == NULL) {
        return -1;
    }

    memset(out_commit, 0, sizeof(*out_commit));
    (void)snprintf(out_commit->id, sizeof(out_commit->id), "%s", commit_id);
    out_commit->files = NULL;

    line = strtok(content, "\n");
    while (line != NULL) {
        if (strncmp(line, "parent:", 7) == 0) {
            (void)snprintf(out_commit->parent, sizeof(out_commit->parent), "%s", line + 7);
        } else if (strncmp(line, "timestamp:", 10) == 0) {
            out_commit->timestamp = strtol(line + 10, NULL, 10);
        } else if (strncmp(line, "message:", 8) == 0) {
            (void)snprintf(out_commit->message, sizeof(out_commit->message), "%s", line + 8);
        } else if (strchr(line, '|') != NULL) {
            char *sep = strchr(line, '|');
            *sep = '\0';
            append_file_entry(&out_commit->files, line, sep + 1);
        }

        line = strtok(NULL, "\n");
    }

    free(content);
    return 0;
}

void free_commit(Commit *commit) {
    if (commit == NULL) {
        return;
    }
    free_file_list(commit->files);
    commit->files = NULL;
}

int print_commit_log(void) {
    char current[32];

    if (!repo_exists()) {
        fprintf(stderr, "Not a mygit repository. Run 'mygit init' first.\n");
        return -1;
    }

    if (repo_read_head(current, sizeof(current)) != 0 || strcmp(current, "none") == 0) {
        printf("No commits yet.\n");
        return 0;
    }

    while (strcmp(current, "none") != 0) {
        Commit commit;
        if (load_commit(current, &commit) != 0) {
            fprintf(stderr, "Broken commit chain at %s\n", current);
            return -1;
        }

        printf("commit %s\n", commit.id);
        printf("message: %s\n", commit.message);
        printf("parent: %s\n", strlen(commit.parent) > 0 ? commit.parent : "none");
        printf("timestamp: %ld\n\n", commit.timestamp);

        (void)snprintf(current, sizeof(current), "%s", strlen(commit.parent) > 0 ? commit.parent : "none");
        free_commit(&commit);
    }

    return 0;
}

int checkout_commit(const char *commit_id) {
    Commit commit;
    FileEntry *entry;

    if (!repo_exists()) {
        fprintf(stderr, "Not a mygit repository. Run 'mygit init' first.\n");
        return -1;
    }

    if (load_commit(commit_id, &commit) != 0) {
        fprintf(stderr, "Commit not found: %s\n", commit_id);
        return -1;
    }

    entry = commit.files;
    while (entry != NULL) {
        char object_path[512];
        (void)snprintf(object_path, sizeof(object_path), "%s/%s.txt", OBJECTS_DIR, entry->hash);
        if (copy_file(object_path, entry->filename) != 0) {
            fprintf(stderr, "Failed to restore file %s\n", entry->filename);
            free_commit(&commit);
            return -1;
        }
        entry = entry->next;
    }

    if (repo_write_head(commit.id) != 0) {
        fprintf(stderr, "Warning: checkout restored files but failed to update HEAD\n");
    }

    printf("Checked out commit %s\n", commit.id);
    free_commit(&commit);
    return 0;
}