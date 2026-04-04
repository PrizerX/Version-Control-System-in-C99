#include "commit.h"
#include "diff.h"
#include "queue.h"
#include "repo.h"
#include "stack.h"

#include <stdio.h>
#include <string.h>

/* Prints CLI usage help text. */
static void print_usage(void) {
    printf("prk - mini version control system (C99)\n\n");
    printf("Usage:\n");
    printf("  prk init\n");
    printf("  prk add <filename>\n");
    printf("  prk commit \"<message>\"\n");
    printf("  prk log\n");
    printf("  prk checkout <commit_id>\n");
    printf("  prk diff <commit1> <commit2>\n");
    printf("  prk undo\n");
}

int main(int argc, char *argv[]) {
    Stack commit_stack;
    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "init") == 0) {
        return repo_init() == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "add") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: prk add <filename>\n");
            return 1;
        }
        return repo_enqueue_file(argv[2]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "commit") == 0) {
        char commit_id[32];
        if (argc != 3) {
            fprintf(stderr, "Usage: prk commit \"<message>\"\n");
            return 1;
        }
        if (build_commit_stack(&commit_stack) != 0) {
            fprintf(stderr, "Failed to initialize commit stack\n");
            return 1;
        }
        return create_commit(argv[2], &commit_stack, commit_id, sizeof(commit_id)) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "log") == 0) {
        return print_commit_log() == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "checkout") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: prk checkout <commit_id>\n");
            return 1;
        }
        return checkout_commit(argv[2]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "diff") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: prk diff <commit1> <commit2>\n");
            return 1;
        }
        return diff_commits(argv[2], argv[3]) == 0 ? 0 : 1;
    }

    if (strcmp(argv[1], "undo") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Usage: prk undo\n");
            return 1;
        }
        if (build_commit_stack(&commit_stack) != 0) {
            fprintf(stderr, "Failed to initialize commit stack\n");
            return 1;
        }
        return undo_last_commit(&commit_stack) == 0 ? 0 : 1;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_usage();
    return 1;
}