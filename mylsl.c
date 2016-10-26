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
#include <pwd.h>
#include <grp.h>

/*
 * mylsl() - produce the appropriate directory listing(s)
 */
char ** curFiles;
int numCurFiles;
char * currentPath;

void print(){
    int i;
    
    for(i = 0; i < numCurFiles; i++){
        struct stat fileInfo;
        char filePath[1024];
        filePath[0] = '\0';
        strcat(filePath, currentPath);
        strcat(filePath, curFiles[i]);
        if(stat(filePath, &fileInfo) >= 0){
            char permissions[10];
            permissions[0]= '\0';
            strcat(permissions, (fileInfo.st_mode & S_IRUSR) ? "r" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IWUSR) ? "w" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IXUSR) ? "x" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IRGRP) ? "r" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IWGRP) ? "w" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IXGRP) ? "x" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IROTH) ? "r" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IWOTH) ? "w" : "-");
            strcat(permissions, (fileInfo.st_mode & S_IXOTH) ? "x" : "-");

            //printf("%s. ", permissions);
            
            
            unsigned int hardLinks = fileInfo.st_nlink;
            
            struct passwd * ownerInfo;
            uid_t ownerID = fileInfo.st_uid;
            ownerInfo = getpwuid(ownerID);
            char * ownerName = ownerInfo->pw_name;

            struct group * groupInfo;
            gid_t groupOwnerID = fileInfo.st_gid;
            groupInfo = getgrgid(groupOwnerID);
            char * groupName = groupInfo->gr_name;

            off_t fileSize = fileInfo.st_size;

            time_t modTime = fileInfo.st_mtime;

            printf("%s. %u %s %s %ld ", permissions, hardLinks, ownerName, groupName, fileSize);
            

            if(S_ISDIR(fileInfo.st_mode)){
                printf("%s/\n", curFiles[i]);
            }else if(fileInfo.st_mode & S_IXUSR){
                printf("%s*\n", curFiles[i]);
            }else{
                printf("%s\n", curFiles[i]);
            }
        }else{
            //printf("%s\n", filePath);
            perror("Failed to stat file\n");
        }
        
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
    printf("compare returning 0\n");
    //the words are alphabetically the same
    return 0;

    //int result = strcmp((* (char * const *)(word1)), (* (char * const *)(word2)));
    //return result;
}

void mylslHelper(char * dirName){

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
    }else if(getcwd(temp, sizeof(temp))){   //we werent given a directory so we default to the current dir
        currPath = malloc((strlen(temp) + 2) * sizeof(char));
        strcpy(currPath, temp);
        strcat(currPath, "/");
        //currPath = strdup(temp);
        //printf("%s\n", currPath);
        //printf("Current Path: %s\n", currPath);
    }else{                                  //we are unable to get the path ERROR
        printf("Path error!\n");
        exit(1);
    }
    currentPath = strdup(currPath);
    //setting up directory and incremental variables
    DIR* myDir;
    struct dirent * entry;
    int buffinc = 1;
    curFiles = malloc(1024 * sizeof(char *));
    numCurFiles = 0;

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
            exit(1);
        }
        
    }
    

    qsort(curFiles, numCurFiles, sizeof(char *), myCompare);
    print();
    int i;
    for(i = 0; i < numCurFiles; i++){
        free(curFiles[i]);
    }
    free(curFiles);
    free(currPath);
    free(currentPath);
}

//takes in all arguments and calls an ls on each default is current directory
void mylsl(char **roots, int num) { 
 
    if(num == 0){
        mylslHelper(NULL);
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
        mylslHelper(files[i]);
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
            mylslHelper(dirs[i]);
            printf("\n");
        }else{
            mylslHelper(dirs[i]);
        }
        free(dirs[i]);
    }
    free(dirs);

}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [FILE]...\n", progname);
    printf("List information about the FILEs (the current directory by default).\n");
    printf("Use a long listing format\n");
    printf("Behavior should mirror /bin/ls -1 -l\n");
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
        if(argc > 1){
        mylsl(++argv, --argc);
    }else{
         mylsl(NULL, 0);
    }
}
