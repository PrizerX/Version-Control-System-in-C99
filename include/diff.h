#ifndef DIFF_H
#define DIFF_H

/* Compares two commits and prints line-by-line differences. */
int diff_commits(const char *commit1, const char *commit2);

#endif