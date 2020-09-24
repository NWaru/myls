#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "output.h"
#include "utility.h"

char* getFilePermissions(struct stat* dirinfo) {
    char buf[] = "----------";
    char* filePerm = malloc(11*sizeof(char));
    mode_t mode = dirinfo->st_mode;

    if(S_ISDIR(mode)) {
        buf[0] = 'd';
    }  
    if(S_ISLNK(mode)) {
       buf[0] = 'l'; 
    } 
    if(mode & S_IRUSR) {
        buf[1] = 'r';
    }
    if(mode & S_IWUSR) {
        buf[2] = 'w';
    }
    if(mode & S_IXUSR) {
        buf[3] = 'x';
    }
    if(mode & S_IRGRP) {
        buf[4] = 'r';
    }
    if(mode & S_IWGRP) {
        buf[5] = 'w';
    }
    if(mode & S_IXGRP) {
        buf[6] = 'x';
    }
    if(mode & S_IROTH) {
        buf[7] = 'r';
    }
    if(mode & S_IWOTH) {
        buf[8] = 'w';
    }
    if(mode & S_IXOTH) {
        buf[9] = 'x';
    }
    
    strcpy(filePerm, buf);
    return filePerm;

}

void outputRegList(int printInode, int numfiles, struct dirent** dplist, 
        struct stat** dirInfoList, char* path, int is_cwd, struct padding_t* pp) {
    char* tmp; 
    char* name; 

    for(int i = 0; i < numfiles; i++) {
        if(is_cwd) {
            name = add_quotes(dplist[i]->d_name); 
        } else {
            tmp = get_full_path(path, dplist[i]->d_name);
            name = add_quotes(tmp);
            free(tmp);
        }

        if(printInode) {
            printf("%*llu %s\n", pp->inode_count, (unsigned long long)dirInfoList[i]->st_ino, name);
        } else {
            printf("%s\n", name); 
        }

        free(name); 
    }
}

void outputLongList(int printInode, int numfiles, struct dirent** dplist,
        struct stat** dirInfoList, char* path, int is_cwd, struct padding_t* pp) {

   char* monthList[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; 

   for(int i = 0; i < numfiles; i++) {
        struct passwd *pw = getpwuid(dirInfoList[i]->st_uid); 
        struct group *grp = getgrgid(dirInfoList[i]->st_gid); 
        struct tm *ts;
        ts = localtime(&(dirInfoList[i]->st_mtime));

        char* fileperm = getFilePermissions(dirInfoList[i]);
        unsigned int numlinks = (unsigned int)dirInfoList[i]->st_nlink;
        char* uid = pw->pw_name;
        char* gid = grp->gr_name; 
        unsigned long size = (unsigned long)dirInfoList[i]->st_size; 
        char* month = monthList[(ts->tm_mon)];
        int day = ts->tm_mday; 
        int year = (ts->tm_year) + 1900;  
        int hour = ts->tm_hour; 
        int min = ts->tm_min;

        char* name; 

        if(fileperm[0] == 'l') {
            char buf[100];
            char* tmp1; 
            char* tmp2;
            int len;  

            char* fullpath = get_full_path(path, dplist[i]->d_name);
            len = readlink(fullpath, buf, sizeof(buf)); 
            buf[len] = '\0'; 
            tmp1 = add_quotes(dplist[i]->d_name);
            tmp2 = add_quotes(buf);

            name = malloc(strlen(tmp1) + strlen(tmp2) + 5);

            strcpy(name, tmp1);
            strcat(name, " -> "); 
            strcat(name, tmp2); 

            free(fullpath);
            free(tmp1); 
            free(tmp2); 
        } else if(is_cwd){
            name = add_quotes(dplist[i]->d_name);
        } else {
            char* tmp;
            tmp = get_full_path(path, dplist[i]->d_name);
            name = add_quotes(tmp);

            free(tmp);
        }
            
        if(printInode) {
            printf("%*llu %s %*u %*s %*s %*lu %s %2d %4d %02d:%02d %s\n", 
                    pp->inode_count, (unsigned long long)dirInfoList[i]->st_ino, 
                    fileperm, pp->ref_count, numlinks, 
                    pp->usr_count, uid, pp->group_count, gid, pp->size_count, size, 
                    month, day, year, hour, min, 
                    name);
        } else {
            printf("%s %*u %*s %*s %*lu %s %2d %4d %02d:%02d %s\n",
                    fileperm, pp->ref_count, numlinks,
                    pp->usr_count, uid, pp->group_count, gid, pp->size_count, size,
                    month, day, year, hour, min,
                    name);
        }

        free(name);
        free(fileperm); 
   } 
}

        

