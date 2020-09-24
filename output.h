#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <dirent.h>
#include <sys/stat.h>

struct padding_t {
    int inode_count;
    int ref_count;
    int group_count;
    int usr_count;
    int size_count;
};

char* getFilePermissions(struct stat* dirinfo);
void outputRegList(int printInode, int numfiles, struct dirent** dplist, 
        struct stat** dirInfoList, char* path, int is_cwd, struct padding_t* pp);
void outputLongList(int printInode, int numfiles, struct dirent** dplist, 
        struct stat** dirInfoList, char* path, int is_cwd, struct padding_t* pp); 

#endif
