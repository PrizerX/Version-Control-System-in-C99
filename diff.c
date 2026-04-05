#include "diff.h"

#include "commit.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OBJECTS_DIR ".prk/objects"
#define MAX_DIFF_LINES 1000

typedef struct LineArray {
    char **items;
    int count;
} LineArray;

typedef struct DiffOp {
    char type;
    const char *text;
} DiffOp;

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

static char *duplicate_range(const char *start, size_t len) {
    char *copy = (char *)malloc(len + 1U);
    if (copy == NULL) {
        return NULL;
    }
    if (len > 0U) {
        memcpy(copy, start, len);
    }
    copy[len] = '\0';
    return copy;
}

static void free_line_array(LineArray *lines) {
    int i;

    if (lines == NULL || lines->items == NULL) {
        return;
    }

    for (i = 0; i < lines->count; ++i) {
        free(lines->items[i]);
    }
    free(lines->items);
    lines->items = NULL;
    lines->count = 0;
}

static int split_lines(const char *text, LineArray *out_lines) {
    const char *cursor;
    int capacity = 16;

    if (out_lines == NULL) {
        return -1;
    }

    out_lines->items = (char **)malloc((size_t)capacity * sizeof(char *));
    out_lines->count = 0;
    if (out_lines->items == NULL) {
        return -1;
    }

    cursor = text != NULL ? text : "";
    while (*cursor != '\0') {
        const char *line_start = cursor;
        const char *line_end;
        size_t len;
        char *line_copy;

        while (*cursor != '\0' && *cursor != '\n') {
            cursor++;
        }
        line_end = cursor;
        len = (size_t)(line_end - line_start);

        if (out_lines->count >= MAX_DIFF_LINES) {
            fprintf(stderr, "Diff limit exceeded: max %d lines\n", MAX_DIFF_LINES);
            free_line_array(out_lines);
            return -1;
        }

        if (out_lines->count >= capacity) {
            int new_capacity = capacity * 2;
            char **new_items;

            if (new_capacity > MAX_DIFF_LINES) {
                new_capacity = MAX_DIFF_LINES;
            }

            new_items = (char **)realloc(out_lines->items, (size_t)new_capacity * sizeof(char *));
            if (new_items == NULL) {
                free_line_array(out_lines);
                return -1;
            }

            out_lines->items = new_items;
            capacity = new_capacity;
        }

        line_copy = duplicate_range(line_start, len);
        if (line_copy == NULL) {
            free_line_array(out_lines);
            return -1;
        }

        out_lines->items[out_lines->count++] = line_copy;

        if (*cursor == '\n') {
            cursor++;
        }
    }

    return 0;
}

static int build_lcs_table(const LineArray *left, const LineArray *right, int **out_dp) {
    int rows;
    int cols;
    int *dp;
    int i;
    int j;

    rows = left->count + 1;
    cols = right->count + 1;
    dp = (int *)calloc((size_t)rows * (size_t)cols, sizeof(int));
    if (dp == NULL) {
        return -1;
    }

    for (i = 1; i < rows; ++i) {
        for (j = 1; j < cols; ++j) {
            if (strcmp(left->items[i - 1], right->items[j - 1]) == 0) {
                dp[i * cols + j] = dp[(i - 1) * cols + (j - 1)] + 1;
            } else {
                int up = dp[(i - 1) * cols + j];
                int left_val = dp[i * cols + (j - 1)];
                dp[i * cols + j] = up >= left_val ? up : left_val;
            }
        }
    }

    *out_dp = dp;
    return 0;
}

static int backtrack_diff_ops(const LineArray *left, const LineArray *right, const int *dp, DiffOp **out_ops, int *out_count) {
    int cols = right->count + 1;
    int i = left->count;
    int j = right->count;
    int capacity = left->count + right->count + 2;
    int count = 0;
    DiffOp *ops = (DiffOp *)malloc((size_t)capacity * sizeof(DiffOp));

    if (ops == NULL) {
        return -1;
    }

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && strcmp(left->items[i - 1], right->items[j - 1]) == 0) {
            ops[count].type = '=';
            ops[count].text = left->items[i - 1];
            count++;
            i--;
            j--;
        } else if (j > 0 && (i == 0 || dp[i * cols + (j - 1)] >= dp[(i - 1) * cols + j])) {
            ops[count].type = '+';
            ops[count].text = right->items[j - 1];
            count++;
            j--;
        } else {
            ops[count].type = '-';
            ops[count].text = left->items[i - 1];
            count++;
            i--;
        }
    }

    {
        int left_idx = 0;
        int right_idx = count - 1;
        while (left_idx < right_idx) {
            DiffOp tmp = ops[left_idx];
            ops[left_idx] = ops[right_idx];
            ops[right_idx] = tmp;
            left_idx++;
            right_idx--;
        }
    }

    *out_ops = ops;
    *out_count = count;
    return 0;
}

static void print_lcs_diff(const char *name, const char *left_text, const char *right_text) {
    LineArray left_lines;
    LineArray right_lines;
    int *dp = NULL;
    DiffOp *ops = NULL;
    int op_count = 0;
    int i;
    int line_no = 1;

    left_lines.items = NULL;
    left_lines.count = 0;
    right_lines.items = NULL;
    right_lines.count = 0;

    if (split_lines(left_text, &left_lines) != 0) {
        return;
    }
    if (split_lines(right_text, &right_lines) != 0) {
        free_line_array(&left_lines);
        return;
    }
    if (build_lcs_table(&left_lines, &right_lines, &dp) != 0) {
        free_line_array(&left_lines);
        free_line_array(&right_lines);
        return;
    }
    if (backtrack_diff_ops(&left_lines, &right_lines, dp, &ops, &op_count) != 0) {
        free(dp);
        free_line_array(&left_lines);
        free_line_array(&right_lines);
        return;
    }

    printf("--- %s\n", name);
    for (i = 0; i < op_count; ++i) {
        if (ops[i].type == '=') {
            line_no++;
            continue;
        }

        if (i + 1 < op_count && ops[i].type == '-' && ops[i + 1].type == '+') {
            printf("line %d\n", line_no);
            printf("- %s\n", ops[i].text);
            printf("+ %s\n", ops[i + 1].text);
            line_no++;
            i++;
            continue;
        }

        if (i + 1 < op_count && ops[i].type == '+' && ops[i + 1].type == '-') {
            printf("line %d\n", line_no);
            printf("+ %s\n", ops[i].text);
            printf("- %s\n", ops[i + 1].text);
            line_no++;
            i++;
            continue;
        }

        printf("line %d\n", line_no);
        if (ops[i].type == '-') {
            printf("- %s\n", ops[i].text);
            line_no++;
        } else if (ops[i].type == '+') {
            printf("+ %s\n", ops[i].text);
        }
    }

    free(ops);
    free(dp);
    free_line_array(&left_lines);
    free_line_array(&right_lines);
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

            print_lcs_diff(it->filename, left, right);

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