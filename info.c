#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "info.h"
#include "utility.h" 

char* filename; 

int selectfile(const struct dirent* dp) {
   return(strcmp((dp->d_name), filename) == 0);
}  
int selectdir(const struct dirent* dp) {
    return((dp->d_name)[0] != '.'); 
}

int getDirEntries(char* directory, char** path, struct dirent*** dplist) {
    int numfiles = 0; 

    if(path != NULL && is_regular_file(*path)) {
        int idx = strlen(*path) - 1; 
        int filename_len = 0; 
        while((*path)[idx] != '/') {
            (*path)[idx] = '\0';  
            filename_len++; 
            idx--; 
        } 
        if(idx != 0) {
            (*path)[idx] = '\0';
        }


        filename = malloc(filename_len + 1); 
        
        idx = filename_len;
        filename[idx] = '\0'; 
        idx--; 
        
        int file_idx = strlen(directory) - 1; 
        while(file_idx >= 0 && directory[file_idx] != '/') {
            filename[idx] = directory[file_idx]; 
            idx--; 
            file_idx--;
        }

        if((numfiles = scandir(*path, dplist, &selectfile, alphasort)) == -1) {
            perror("myls: failed to scan file");
            exit(1);
        }

        free(filename);
    } else { 
        if((numfiles = scandir(directory, dplist, &selectdir, alphasort)) == -1) {
            printf("dir: %s\n", directory);
            perror("myls: failed to scan directory");
            exit(1);
        }
    }

    return numfiles; 
}

struct stat** getDirInfo(struct dirent** dplist, char* path, int numfiles) {
    struct stat** dirInfoList;
    
    if((dirInfoList = calloc(numfiles, sizeof(struct stat*))) == NULL) {
	    perror("myls: failed to allocate space for struct stat list");
	    exit(1); 
    }

    for(int i = 0; i < numfiles; i++) {
        char* fullpath;

        if(path != NULL) {
            fullpath = get_full_path(path, dplist[i]->d_name);
        } else { 
            fullpath = dplist[i]->d_name;
        }

	    dirInfoList[i] = calloc(1,sizeof(struct stat)); 
	    lstat(fullpath, dirInfoList[i]);

        if(path != NULL) {
            free(fullpath);
        }            
    }

    return dirInfoList; 
}
