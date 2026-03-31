#include "commit.h"
#include "diff.h"
#include "repo.h"

#include <stdio.h>
#include <string.h>

/* Prints CLI usage help text. */
static void print_usage(void) {
    printf("mygit - mini version control system (C99)\n\n");
    printf("Usage:\n");
    printf("  mygit init\n");
    printf("  mygit add <filename>\n");
    printf("  mygit commit \"<message>\"\n");
    printf("  mygit log\n");
    printf("  mygit checkout <commit_id>\n");
    printf("  mygit diff <commit1> <commit2>\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "init") == 0) {
        return repo_init() == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: mygit add <filename>\n");
            return 1;
        }
        return repo_add_file(argv[2]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "commit") == 0) {
        char commit_id[32];
        if (argc != 3) {
            fprintf(stderr, "Usage: mygit commit \"<message>\"\n");
            return 1;
        }
        return create_commit(argv[2], commit_id, sizeof(commit_id)) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "log") == 0) {
        return print_commit_log() == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "checkout") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: mygit checkout <commit_id>\n");
            return 1;
        }
        return checkout_commit(argv[2]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "diff") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: mygit diff <commit1> <commit2>\n");
            return 1;
        }
        return diff_commits(argv[2], argv[3]) == 0 ? 0 : 1;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_usage();
    return 1;
}