#ifndef _INFO_H_
#define _INFO_H_

#include <sys/stat.h>

int selectdir(const struct dirent* dp); 
int getDirEntries(char* directory, char** path, struct dirent*** dplist);
struct stat** getDirInfo(struct dirent** dplist, char* path, int numfiles);

#endif
