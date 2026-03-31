#include "diff.h"

#include "commit.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OBJECTS_DIR ".mygit/objects"

static FileEntry *build_union(FileEntry *a, FileEntry *b) {
    FileEntry *union_list = NULL;
    FileEntry *it;

    it = a;
    while (it != NULL) {
        append_file_entry(&union_list, it->filename, "");
        it = it->next;
    }

    it = b;
    while (it != NULL) {
        if (find_file_entry(union_list, it->filename) == NULL) {
            append_file_entry(&union_list, it->filename, "");
        }
        it = it->next;
    }

    return union_list;
}

static const char *next_line_ptr(const char **cursor, char *buffer, size_t size) {
    size_t i = 0;
    const char *src = *cursor;

    if (src == NULL || *src == '\0') {
        return NULL;
    }

    while (*src != '\0' && *src != '\n' && i + 1U < size) {
        buffer[i++] = *src++;
    }
    buffer[i] = '\0';

    if (*src == '\n') {
        src++;
    }
    *cursor = src;
    return buffer;
}

static void print_line_diff(const char *name, const char *left, const char *right) {
    const char *p1 = left != NULL ? left : "";
    const char *p2 = right != NULL ? right : "";
    char line1[1024];
    char line2[1024];
    int line_no = 1;

    printf("--- %s\n", name);
    while (*p1 != '\0' || *p2 != '\0') {
        const char *l1 = next_line_ptr(&p1, line1, sizeof(line1));
        const char *l2 = next_line_ptr(&p2, line2, sizeof(line2));

        if (l1 == NULL) {
            l1 = "";
        }
        if (l2 == NULL) {
            l2 = "";
        }

        if (strcmp(l1, l2) != 0) {
            printf("line %d\n", line_no);
            printf("- %s\n", l1);
            printf("+ %s\n", l2);
        }
        line_no++;
    }
}

int diff_commits(const char *commit1, const char *commit2) {
    Commit c1;
    Commit c2;
    FileEntry *names;
    FileEntry *it;

    if (load_commit(commit1, &c1) != 0) {
        fprintf(stderr, "Commit not found: %s\n", commit1);
        return -1;
    }
    if (load_commit(commit2, &c2) != 0) {
        fprintf(stderr, "Commit not found: %s\n", commit2);
        free_commit(&c1);
        return -1;
    }

    names = build_union(c1.files, c2.files);
    it = names;
    while (it != NULL) {
        FileEntry *f1 = find_file_entry(c1.files, it->filename);
        FileEntry *f2 = find_file_entry(c2.files, it->filename);
        const char *h1 = f1 != NULL ? f1->hash : "";
        const char *h2 = f2 != NULL ? f2->hash : "";

        if (strcmp(h1, h2) != 0) {
            char p1[512];
            char p2[512];
            char *left = NULL;
            char *right = NULL;

            if (strlen(h1) > 0) {
                (void)snprintf(p1, sizeof(p1), "%s/%s.txt", OBJECTS_DIR, h1);
                left = read_text_file(p1);
            }
            if (strlen(h2) > 0) {
                (void)snprintf(p2, sizeof(p2), "%s/%s.txt", OBJECTS_DIR, h2);
                right = read_text_file(p2);
            }

            print_line_diff(it->filename, left, right);

            free(left);
            free(right);
        }

        it = it->next;
    }

    free_file_list(names);
    free_commit(&c1);
    free_commit(&c2);
    return 0;
}