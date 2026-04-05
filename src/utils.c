#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

int path_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

int ensure_dir(const char *path) {
    if (path_exists(path)) {
        return 0;
    }
#ifdef _WIN32
    if (_mkdir(path) != 0) {
#else
    if (mkdir(path, 0777) != 0) {
#endif
        return -1;
    }
    return 0;
}

char *read_text_file(const char *path) {
    FILE *fp;
    long size;
    size_t read_size;
    char *buffer;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    buffer = (char *)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }

    read_size = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);

    if (read_size != (size_t)size) {
        free(buffer);
        return NULL;
    }
    buffer[size] = '\0';
    return buffer;
}

int write_text_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "wb");
    size_t len;

    if (fp == NULL) {
        return -1;
    }

    len = strlen(content);
    if (len > 0 && fwrite(content, 1, len, fp) != len) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int copy_file(const char *src, const char *dst) {
    FILE *in;
    FILE *out;
    char buffer[4096];
    size_t n;

    in = fopen(src, "rb");
    if (in == NULL) {
        return -1;
    }

    out = fopen(dst, "wb");
    if (out == NULL) {
        fclose(in);
        return -1;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return -1;
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}

char *duplicate_string(const char *input) {
    size_t len;
    char *copy;

    if (input == NULL) {
        return NULL;
    }
    len = strlen(input);
    copy = (char *)malloc(len + 1U);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, input, len + 1U);
    return copy;
}

void trim_newline(char *text) {
    size_t len;

    if (text == NULL) {
        return;
    }
    len = strlen(text);
    while (len > 0 && (text[len - 1U] == '\n' || text[len - 1U] == '\r')) {
        text[len - 1U] = '\0';
        --len;
    }
}

FileEntry *append_file_entry(FileEntry **head, const char *filename, const char *hash) {
    FileEntry *node;
    FileEntry *current;

    if (head == NULL || filename == NULL || hash == NULL) {
        return NULL;
    }

    node = (FileEntry *)malloc(sizeof(FileEntry));
    if (node == NULL) {
        return NULL;
    }

    (void)snprintf(node->filename, sizeof(node->filename), "%s", filename);
    (void)snprintf(node->hash, sizeof(node->hash), "%s", hash);
    node->next = NULL;

    if (*head == NULL) {
        *head = node;
        return node;
    }

    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
    return node;
}

FileEntry *find_file_entry(FileEntry *head, const char *filename) {
    while (head != NULL) {
        if (strcmp(head->filename, filename) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

void free_file_list(FileEntry *head) {
    FileEntry *next;
    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}