#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"

/*
 * myrgrep() - find files (recursively) with matching pattern
 */
void myrgrep(char *pattern, char **roots) {
    /* TODO: Complete this function */
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [OPTIONS] [PATTERN] [FILE]...\n", progname);
    printf("List the names of the FILEs (the current directory by default)\n");
    printf("whose contents contain PATTERN.\n");
    printf("The search should be recursive into subdirectories of any FILE\n");
}

/*
 * main() - The main routine parses arguments and dispatches to the
 *          task-specific code.
 */
int main(int argc, char **argv) {
    /* for getopt */
    long opt;

    /* run a student name check */
    check_student(argv[0]);

    /* parse the command-line options.  For this program, we only support  */
    /* the parameterless 'h' option, for getting help on program usage. */
    /* TODO: make sure to handle any other arguments specified by the */
    /*       assignment */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch(opt) {
          case 'h': help(argv[0]); break;
        }
    }

    /* TODO: fix this invocation */
    myrgrep(NULL, NULL);
}
