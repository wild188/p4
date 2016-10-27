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
#include <errno.h>
#include <sys/stat.h>

/*
 * myrgrep() - find files (recursively) with matching pattern
 */

 char * searchString;
char * prefix;
int root;

int grep_file(char *filename, char *searchstr) {
  //printf("grepping %s for %s\n", filename ,searchstr);
  FILE *file = fopen(filename, "r");
  if(!file){
    return 0;
  }

  char line[1024];
  while(fgets(line, 1024, file)){
    if(strstr(line, searchstr) != NULL){
      //printf("%s", line);
       fclose(file);
      return 1;
    }
  }
  fclose(file);
  return 0;
}

void print(char ** curFiles, int numCurFiles, char * currentPath){
    int i;
    //printf("printing\n");
    for(i = 0; i < numCurFiles; i++){
        struct stat fileInfo;
        char filePath[1024];
        filePath[0] = '\0';
        int len = strlen(currentPath);
        strcat(filePath, currentPath);
        if(filePath[len-1] != '/'){
            strcat(filePath, "/");
        }
        strcat(filePath, curFiles[i]);
        if(stat(filePath, &fileInfo) == 0){
            if(!grep_file(filePath, searchString)){
                continue;
            }
            printf("%s\n", filePath);
            /*
            if(S_ISDIR(fileInfo.st_mode)){
                printf("%s/\n", curFiles[i]);
            }else if(fileInfo.st_mode & S_IXUSR){
                printf("%s*\n", curFiles[i]);
            }else{
                printf("%s\n", curFiles[i]);
            }
            */
        }else{
            printf("%s\n", filePath);
            perror("Failed to stat file\n");
        }
        
    }
}

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
    //printf("compare returning 0\n");
    //the words are alphabetically the same
    return 0;

    //int result = strcmp((* (char * const *)(word1)), (* (char * const *)(word2)));
    //return result;
}


void myrgrepHelper(char * dirName){
    //printf("opening dir %s \n", dirName);

    //printf("entering old myls\n");

    //getting the path to the directory we will analyze
    char * currPath;
    char temp[1024];


    if(dirName != NULL){                   //we were given a directory so we use that one

        currPath = strdup(dirName);
        /*
        int len = strlen(dirName);
        currPath = malloc((len + 1)* (sizeof(char)));
        strcpy(currPath, dirName);
        */
        //printf("Evaluating external directory: %s\n", currPath);
    }else if(0 && getcwd(temp, sizeof(temp))){   //we werent given a directory so we default to the current dir
        currPath = malloc((strlen(temp) + 2) * sizeof(char));
        strcpy(currPath, temp);
        strcat(currPath, "/");
        //currPath = strdup(temp);
        //printf("%s\n", currPath);
        //printf("Current Path: %s\n", currPath);
    }else{
        //currPath = malloc(3 * sizeof(char));
        //currPath = "./";
         currPath = strdup("./");
         
                                          //we are unable to get the path ERROR
        //printf("Path error!\n");
        //exit(1);
    }
    //currentPath = strdup(currPath);
    //setting up directory and incremental variables
    DIR* myDir;
    struct dirent * entry;
    int buffinc = 1;
    char ** curFiles = malloc(1024 * sizeof(char *));
    int numCurFiles = 0;

    char ** subDirs = malloc(1024 * sizeof(char *));
    int numSubDirs = 0;


    if((myDir = opendir(currPath)) != NULL){
        //perror("oppened directory \n");
        while((entry = readdir(myDir)) != NULL){
            char * name = entry->d_name; //gets the file name
            if(name == NULL){ //skips nyll names
                continue;
            }else{
                if(name[0] == '.'){
                    continue; //skipping hidden entries
                }else{
                    numCurFiles++;

                    //increments file buffer if we excede 1024 files
                    if(numCurFiles > (1024 * buffinc)){
                        curFiles = realloc(curFiles, ((numCurFiles + 1024) * sizeof(char *)));
                        buffinc++;
                    }

                    struct stat posDir;
                    char * filePath = malloc((strlen(currPath) + strlen(name) + 2) * sizeof(char));
                    filePath[0] = '\0';
                    strcat(filePath, currPath);
                    if(filePath[strlen(filePath) - 1] != '/'){
                        strcat(filePath, "/");
                    }
                    
                    strcat(filePath, name);
                    if(stat(filePath, &posDir) >= 0){
                        if(S_ISDIR(posDir.st_mode)){

                            //printf("Found sub directory %s\n", filePath);

                            subDirs[numSubDirs] = strdup(filePath);
                            numSubDirs++;
                        }else{
                            //printf("%s is not a directory\n", filePath);
                        }
                    }else{
                        //printf("%s is not a file\n", filePath);
                    }
                    free(filePath);

                    //stores the name of the file in the file list
                    curFiles[numCurFiles -1] = strdup(name);
                }
            }
        }
        closedir(myDir);
    }else{
        if(errno == ENOTDIR){       //if we are given a file
            printf("%s\n", currPath);
        }else{                      //unable to open the directory ERROR
            printf("Problem openning directory.\n");
            return;
            //exit(1);
        }
        
    }
    
    
    qsort(curFiles, numCurFiles, sizeof(char *), myCompare);
    
    
    if(numSubDirs > 0 && root){
        //printf("%s:\n", currPath);
        root = 0;
    }
    

    print(curFiles, numCurFiles, currPath);
    int i;
    for(i = 0; i < numCurFiles; i++){
        free(curFiles[i]);
    }

    

    qsort(subDirs, numSubDirs, sizeof(char *), myCompare);
    for(i = 0; i < numSubDirs; i++){
        char * subPath = malloc((strlen(currPath) + strlen(subDirs[i]) + 1) * sizeof(char));
        subPath[0] = '\0';
        strcat(subPath, currPath);
        strcat(subPath, subDirs[i]);
        //printf("\n%s:\n", subDirs[i]);
        myrgrepHelper(subDirs[i]);
        free(subPath);
        free(subDirs[i]);
    }


    free(curFiles);
    free(currPath);
    free(subDirs);
}

void subdir(char ** curFiles, int numCurFiles, char * currentPath){
    int i;
    for(i = 0; i < numCurFiles; i++){
        DIR* subdir;
        if((subdir = opendir(curFiles[i])) != NULL){
            closedir(subdir);
            prefix = malloc(strlen(curFiles[i]) * sizeof(char));
            //prefix 
            printf("\n");
            myrgrepHelper(curFiles[i]);
        }
    }
}

//takes in all arguments and calls an ls on each default is current directory
void myrgrep(char **roots, int num) { 
    root = 1;
    if(num == 0){
        myrgrepHelper(NULL);
        return;
    }   

    int i;
    char ** files = malloc(1024 *sizeof(char *));
    int numFiles = 0;
    char ** dirs = malloc(1024 * sizeof(char *));
    int numDirs = 0;

    int fileInc = 0;
    int dirInc = 0;
    for(i = 0; i < num; i++){
        struct stat info;
        if(stat(roots[i], &info) == 0){
            if(S_ISDIR(info.st_mode)){
                if(numDirs > (1023 * dirInc)){
                    dirs = realloc(dirs, (numDirs + 1024) * sizeof(char *));
                    dirInc++;
                }
                dirs[numDirs] = strdup(roots[i]);
                numDirs++;
            }else if(S_ISREG(info.st_mode)){
                if(numFiles > (1023 * fileInc)){
                    files = realloc(files, (numFiles + 1024) * sizeof(char *));
                    fileInc++;
                }
                files[numFiles] = strdup(roots[i]);
                numFiles++;
            }
        }else{
            printf("ls: cannot access %s: No such file or directory\n", roots[i]);
        }
    }
    
    qsort(files, numFiles, sizeof(char *), myCompare);
    for(i = 0; i < numFiles; i++){
        myrgrepHelper(files[i]);
        free(files[i]);
    }
    free(files);

    if(numDirs < num){
        printf("\n");
    }

    qsort(dirs, numDirs, sizeof(char *), myCompare);
    for(i = 0; i < numDirs; i++){
        if(numDirs > 1){
            printf("%s:\n", dirs[i]);
            myrgrepHelper(dirs[i]);
            printf("\n");
        }else{
            myrgrepHelper(dirs[i]);
        }
        free(dirs[i]);
    }
    free(dirs);

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
   if(argc == 3){
        searchString = strdup(argv[1]);
        myrgrepHelper(argv[2]);
   }
}
