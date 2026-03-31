#include "repo.h"

#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPO_DIR ".mygit"
#define OBJECTS_DIR ".mygit/objects"
#define COMMITS_DIR ".mygit/commits"
#define REFS_DIR ".mygit/refs"
#define HEAD_FILE ".mygit/HEAD"
#define INDEX_FILE ".mygit/index.txt"

static int read_index_internal(FileEntry **out_head) {
    char *content;
    char *line;
    FileEntry *head = NULL;

    if (out_head == NULL) {
        return -1;
    }

    if (!path_exists(INDEX_FILE)) {
        *out_head = NULL;
        return 0;
    }

    content = read_text_file(INDEX_FILE);
    if (content == NULL) {
        return -1;
    }

    line = strtok(content, "\n");
    while (line != NULL) {
        char *sep = strchr(line, '|');
        if (sep != NULL) {
            *sep = '\0';
            append_file_entry(&head, line, sep + 1);
        }
        line = strtok(NULL, "\n");
    }

    free(content);
    *out_head = head;
    return 0;
}

int repo_init(void) {
    if (ensure_dir(REPO_DIR) != 0) {
        fprintf(stderr, "Failed to create %s\n", REPO_DIR);
        return -1;
    }
    if (ensure_dir(OBJECTS_DIR) != 0) {
        fprintf(stderr, "Failed to create %s\n", OBJECTS_DIR);
        return -1;
    }
    if (ensure_dir(COMMITS_DIR) != 0) {
        fprintf(stderr, "Failed to create %s\n", COMMITS_DIR);
        return -1;
    }
    if (ensure_dir(REFS_DIR) != 0) {
        fprintf(stderr, "Failed to create %s\n", REFS_DIR);
        return -1;
    }

    if (!path_exists(HEAD_FILE) && write_text_file(HEAD_FILE, "none\n") != 0) {
        fprintf(stderr, "Failed to create HEAD file\n");
        return -1;
    }
    if (!path_exists(INDEX_FILE) && write_text_file(INDEX_FILE, "") != 0) {
        fprintf(stderr, "Failed to create index file\n");
        return -1;
    }

    printf("Initialized empty mygit repository in .mygit/\n");
    return 0;
}

int repo_exists(void) {
    return path_exists(REPO_DIR) && path_exists(OBJECTS_DIR) && path_exists(COMMITS_DIR) && path_exists(HEAD_FILE);
}

int repo_read_index(FileEntry **out_head) {
    return read_index_internal(out_head);
}

int repo_write_index(FileEntry *head) {
    FILE *fp = fopen(INDEX_FILE, "wb");
    if (fp == NULL) {
        return -1;
    }

    while (head != NULL) {
        if (fprintf(fp, "%s|%s\n", head->filename, head->hash) < 0) {
            fclose(fp);
            return -1;
        }
        head = head->next;
    }

    fclose(fp);
    return 0;
}

int repo_read_head(char *out_commit, size_t out_size) {
    char *content;

    if (out_commit == NULL || out_size == 0) {
        return -1;
    }

    content = read_text_file(HEAD_FILE);
    if (content == NULL) {
        return -1;
    }

    trim_newline(content);
    (void)snprintf(out_commit, out_size, "%s", content);
    free(content);
    return 0;
}

int repo_write_head(const char *commit_id) {
    char buffer[128];
    (void)snprintf(buffer, sizeof(buffer), "%s\n", commit_id);
    return write_text_file(HEAD_FILE, buffer);
}

int repo_add_file(const char *filename) {
    char *content;
    char hash[32];
    char object_path[512];
    FileEntry *index_head = NULL;
    FileEntry *existing;
    int rc = -1;

    if (!repo_exists()) {
        fprintf(stderr, "Not a mygit repository. Run 'mygit init' first.\n");
        return -1;
    }

    content = read_text_file(filename);
    if (content == NULL) {
        fprintf(stderr, "Unable to read file: %s\n", filename);
        return -1;
    }

    hash_buffer_hex((const unsigned char *)content, strlen(content), hash, sizeof(hash));
    (void)snprintf(object_path, sizeof(object_path), "%s/%s.txt", OBJECTS_DIR, hash);

    if (write_text_file(object_path, content) != 0) {
        fprintf(stderr, "Unable to store object for file: %s\n", filename);
        free(content);
        return -1;
    }
    free(content);

    if (repo_read_index(&index_head) != 0) {
        fprintf(stderr, "Unable to read index\n");
        return -1;
    }

    existing = find_file_entry(index_head, filename);
    if (existing != NULL) {
        (void)snprintf(existing->hash, sizeof(existing->hash), "%s", hash);
    } else if (append_file_entry(&index_head, filename, hash) == NULL) {
        fprintf(stderr, "Out of memory while updating index\n");
        free_file_list(index_head);
        return -1;
    }

    if (repo_write_index(index_head) != 0) {
        fprintf(stderr, "Unable to write index\n");
    } else {
        printf("Added %s as object %s\n", filename, hash);
        rc = 0;
    }

    free_file_list(index_head);
    return rc;
}