#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"

//added
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>

/*
 * myls() - produce the appropriate directory listing(s)
 */

char ** files;
int numFiles;

void print(){
    int i;
    for(i = 0; i < numFiles; i++){
        printf("%s\n", files[i]);
    }
}

//used to alphabatize teh files
static int myCompare(const void * word1, const void * word2){
    //converting the void pointers to char stars
    char * w1 = *(char * const *)(word1);
    char * w2 = *(char * const *)(word2);

    //incremental variables
    char a, b;
    int i = 0;
    int h = 0;

    //compares each charachter from beginning to end unless unequal
    while(w1[i] && w2[h]){
        a = w1[i];
        b = w2[h];
        
        //ignore non alphanumeric characters
        if(!isalnum(a)){
            i++;
            continue;
        }
        if(!isalnum(b)){
            h++;
            continue;
        }

        //puts all characters to lower case
        a = tolower(a);
        b = tolower(b);
        
        //if characters are not equal return the difference
        if(a != b){
            return (a - b);
        }
        i++;
        h++;
    }

    //return negative or possitive if one word is longer
    if(w1[i]){
        return w1[i];
    }
    if(w2[h]){
        return -w2[h];
    }

    //the words are alphabetically the same
    return 0;

    //int result = strcmp((* (char * const *)(word1)), (* (char * const *)(word2)));
    //return result;
}

//why is roots a char **?
void myls(char **roots) { 
    /* TODO: Complete this function */
    char * currPath;
    char temp[1024];
    if(roots[0] != NULL){
        int len = strlen(roots[0]);
        currPath = malloc(len * (sizeof(char)));
        strcpy(currPath, roots[0]);
        printf("Evaluating external directory: %s\n", currPath);
    }else if(getcwd(temp, sizeof(temp))){
        int len = strlen(temp);
        currPath = malloc(len * (sizeof(char)));
        strcpy(currPath, temp);
        printf("Current Path: %s\n", currPath);
    }else{
        printf("Path error!\n");
        exit(1);
    }
    

    DIR* myDir;
    struct dirent * entry;
    int buffinc = 1;
    files = malloc(1024 * sizeof(char *));
    numFiles = 0;

    if((myDir = opendir(currPath)) != NULL){
        perror("oppened directory \n");
        while((entry = readdir(myDir)) != NULL){
            char * name = entry->d_name;
            if(name == NULL){
                continue;
            }else{
                if(name[0] == '.'){
                    continue; //skipping hidden entries
                }else{
                    numFiles++;

                    if(numFiles > (1024 * buffinc)){
                        files = realloc(files, ((numFiles + 1024) * sizeof(char *)));
                        buffinc++;
                    }

                    //files = realloc(files, (numFiles * sizeof(char *)));
                    files[numFiles -1] = malloc(strlen(name) * sizeof(char));
                    strcpy(files[numFiles - 1], name);
                    //printf("%s \n", name);
                }
            }
        }
    }else{
        printf("Problem openning directory.\n");
    }
    qsort(files, numFiles, sizeof(char *), myCompare);
    print();
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [FILE]...\n", progname);
    printf("List information about the FILEs (the current directory by default).\n");
    printf("Behavior should mirror /bin/ls -1\n");
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
    /* TODO: make sure to handle any other arguments specified by the assignment */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch(opt) {
          case 'h': help(argv[0]); break;
        }
    }

    /* TODO: fix this invocation */

    char * target = NULL;
    if(argc > 1){
        target = argv[1];
        printf("%s\n", target);
    }

    char * pass[1];
    pass[0] = target;

    myls(pass);
}
