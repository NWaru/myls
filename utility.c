#include <stdlib.h>
#include <string.h> 
#include <sys/stat.h>

#include "utility.h"

char* get_full_path(char* path, char* filename) 
{
    char* fullpath = malloc(strlen(path) + strlen(filename) + 2);
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, filename);
            
    return fullpath;
}

char* add_quotes(char* output)
{
    char* result; 
    if(strstr(output, " ") || strstr(output, "$") || strstr(output, "^")
            || strstr(output, "&") || strstr(output, "(") || strstr(output, ")"))
    {
        result = malloc(strlen(output) + 3); 
        strcpy(result, "'"); 
        strcat(result, output); 
        strcat(result, "'");   
    } else {
        result = malloc(strlen(output) + 1);
        strcpy(result, output); 
    }

    return result; 
}

// based on an answer from 
// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int is_regular_file(char* path)
{
    struct stat path_stat;
    lstat(path, &path_stat);

    return (S_ISREG(path_stat.st_mode));
}

int is_directory(char* path)
{
    struct stat path_stat;
    lstat(path, &path_stat);

    return (S_ISDIR(path_stat.st_mode));
}


