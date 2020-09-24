#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

#include "info.h"
#include "output.h"
#include "utility.h"

#define MAX_ARGS 3 

static int opt[MAX_ARGS];    // represents i l R (1 if arg found, else 0)

void get_file_padding(struct padding_t* pp, char** file_list, int file_count)
{
    char buf[50]; 
    pp->inode_count = 0;
    pp->ref_count = 0; 
    pp->group_count = 0;
    pp->usr_count = 0;
    pp->size_count = 0;

    for(int i = 0; i < file_count; i++) {
        struct dirent** dplist;
        char* file = file_list[i];
        char* path = realpath(file, NULL);
        int numfiles = getDirEntries(file, &path, &dplist);
        struct stat** dirInfoList = getDirInfo(dplist, path, numfiles);

        struct passwd *pw = getpwuid(dirInfoList[0]->st_uid);
        struct group *grp = getgrgid(dirInfoList[0]->st_gid);

        char* uid = pw->pw_name;
        char* gid = grp->gr_name;

        sprintf(buf, "%llu", (unsigned long long)dirInfoList[0]->st_ino); 
        if(strlen(buf) > pp->inode_count) {
            pp->inode_count = strlen(buf);
        }

        memset(&buf, 0, 50*sizeof(char));

        sprintf(buf, "%u", (unsigned int)dirInfoList[0]->st_nlink);
        if(strlen(buf) > pp->ref_count) {
            pp->ref_count = strlen(buf) + 1;
        }

        memset(&buf, 0, 50*sizeof(char));

        sprintf(buf, "%lu", (unsigned long)dirInfoList[0]->st_size);
        if(strlen(buf) > pp->size_count) {
            pp->size_count = strlen(buf);
        }
        
        if(strlen(gid) > pp->group_count) {
            pp->group_count = strlen(gid); 
        }
        if(strlen(uid) > pp->usr_count) {
            pp->usr_count = strlen(uid);
        }

        free(path);
        for(int i = 0; i < numfiles; i++) {
            free(dplist[i]);
        }
        free(dplist);

        for(int i = 0; i < numfiles; i++) {
            free(dirInfoList[i]);
        }
        free(dirInfoList);
    }
}

void get_dir_padding(struct padding_t* pp, int numfiles, struct stat** dirInfoList) 
{
    char buf[50];
    pp->inode_count = 0;
    pp->ref_count = 0;
    pp->group_count = 0;
    pp->usr_count = 0;
    pp->size_count = 0;
    
    for(int i = 0; i < numfiles; i++) {
        struct passwd *pw = getpwuid(dirInfoList[0]->st_uid);
        struct group *grp = getgrgid(dirInfoList[0]->st_gid);

        char* uid = pw->pw_name;
        char* gid = grp->gr_name;

        sprintf(buf, "%llu", (unsigned long long)dirInfoList[i]->st_ino);
        if(strlen(buf) > pp->inode_count) {
            pp->inode_count = strlen(buf);
        }

        memset(&buf, 0, 50*sizeof(char));

        sprintf(buf, "%u", (unsigned int)dirInfoList[i]->st_nlink);
        if(strlen(buf) > pp->ref_count) {
            pp->ref_count = strlen(buf);
        }

        memset(&buf, 0, 50*sizeof(char));

        sprintf(buf, "%lu", (unsigned long)dirInfoList[i]->st_size);
        if(strlen(buf) > pp->size_count) {
            pp->size_count = strlen(buf);
        }

        if(strlen(gid) > pp->group_count) {
            pp->group_count = strlen(gid);
        }
        if(strlen(uid) > pp->usr_count) {
            pp->usr_count = strlen(uid);
        }
    }
}

int get_files(int start_idx, int argc, char* argv[], char*** file_list) 
{
    int file_count = 0; 
    int max_file_name = 0; 

    for(int i = start_idx; i < argc; i++) {
        char* path = realpath(argv[i], NULL);

        if(is_regular_file(path)) {
            file_count++;
            if(strlen(argv[i]) > max_file_name) {
                max_file_name = strlen(argv[i]); 
            }
        }
        free(path);
    }

    *file_list = malloc(file_count*sizeof(char*)); 
    for(int i = 0; i < file_count; i++) {
        (*file_list)[i] = malloc(max_file_name + 1); 
    }

    int idx = 0; 
    for(int i = start_idx; i < argc; i++) {
        char* path = realpath(argv[i], NULL);

        if(is_regular_file(path)) {
            strcpy((*file_list)[idx], argv[i]);
            idx++;
        }
        free(path);
    }

    return file_count;
}

int get_dirs(int start_idx, int argc, char* argv[], char*** dir_list) 
{
    int dir_count = 0; 
    int max_dir_name = 0;

    for(int i = start_idx; i < argc; i++) {
        char* path = realpath(argv[i], NULL);

        if(!is_regular_file(path)) {
            dir_count++;
            if(strlen(argv[i]) > max_dir_name) {
                max_dir_name = strlen(argv[i]);
            }
        }
        free(path);
    }

    *dir_list = malloc(dir_count*sizeof(char*));
    for(int i = 0; i < dir_count; i++) {
        (*dir_list)[i] = malloc(max_dir_name +1);
    }

    int idx = 0; 
    for(int i = start_idx; i < argc; i++) {
        char* path = realpath(argv[i], NULL);

        if(!is_regular_file(path)) {
            strcpy((*dir_list)[idx], argv[i]);
            idx++;
        }
        free(path);
    }

    return dir_count;
}

void recursive_arg(char* directory, int init)
{
    struct dirent** dplist;
    char* file = add_quotes(directory);
    char* path = realpath(directory, NULL);

    int numfiles = getDirEntries(directory, &path, &dplist);
    struct stat** dirInfoList = getDirInfo(dplist, path, numfiles);

    struct padding_t* pp = malloc(sizeof(struct padding_t));
    get_dir_padding(pp, numfiles, dirInfoList);
    
    if(!init) {
        printf("\n");
    }
    printf("%s:\n", file);

    if(opt[1]) {
        outputLongList(opt[0], numfiles, dplist, dirInfoList, path, 1, pp);
    } else {
        outputRegList(opt[0], numfiles, dplist, dirInfoList, path, 1, pp);
    }

    for(int i = 0; i < numfiles; i++) {
        char* fullpath = get_full_path(directory, dplist[i]->d_name);
        if(is_directory(fullpath)) {
            recursive_arg(fullpath, 0);
        }
        free(fullpath);
    }
    
    free(file); 
    free(path);
    free(pp);
    for(int i = 0; i < numfiles; i++) {
        free(dplist[i]);
    }
    free(dplist);

    for(int i = 0; i < numfiles; i++) {
        free(dirInfoList[i]);
    }
    free(dirInfoList);
}

int sort_args(const void* arg1, const void* arg2) {
    const char **parg1 = (const char **)arg1;
    const char **parg2 = (const char **)arg2;
    return strcmp(*parg1, *parg2); 
}

int main(int argc, char* argv[]) 
{
    memset(&opt, 0, MAX_ARGS*sizeof(int));

    char* cwd = getcwd(NULL, 0);
    char* format_cwd = realpath(cwd, NULL); 
    char** file_list = NULL;
    char** dir_list = NULL;
    int file_count = 0; 
    int dir_count = 0;  

    int idx = 1; 
    while(idx < argc && argv[idx][0] == '-') {
	    char* curr = argv[idx];
	
	    int flag_idx = 1;
	    while(curr[flag_idx] != '\0') {
	        if(curr[flag_idx] == 'i') {
		        opt[0] = 1; 
	        } else if(curr[flag_idx] == 'l') {
		        opt[1] = 1; 
	        } else if(curr[flag_idx] == 'R') {
		        opt[2] = 1; 
	        } else {
		        fprintf(stderr, "myls: illegal option -- %c\n", curr[flag_idx]);
		        exit(1); 
	        }

	        flag_idx += 1;
	    }
	    
	idx += 1;
    }
    
    // no file/dir args
    if(idx == argc) {
        struct dirent** dplist;
        
        int numfiles = getDirEntries(format_cwd, NULL, &dplist);
        struct stat** dirInfoList = getDirInfo(dplist, NULL, numfiles);

        struct padding_t* pp = malloc(sizeof(struct padding_t));
        get_dir_padding(pp, numfiles, dirInfoList);

        if(opt[2]) {
            recursive_arg(".", 1);
        } else { 
            if(opt[1]) {
                outputLongList(opt[0], numfiles, dplist, dirInfoList, format_cwd, 1, pp);
            } else {
                outputRegList(opt[0], numfiles, dplist, dirInfoList, format_cwd, 1, pp);
            }
        }

        free(pp);

        for(int i = 0; i < numfiles; i++) {
            free(dplist[i]);
        }
        free(dplist);

        for(int i = 0; i < numfiles; i++) {
            free(dirInfoList[i]);
        }
        free(dirInfoList);

    } else {
        // ls on files
        file_list = NULL; 
        dir_list = NULL; 
        file_count = get_files(idx, argc, argv, &file_list);
        dir_count = get_dirs(idx, argc, argv, &dir_list);

        qsort(file_list, file_count, sizeof(char*), &sort_args);
        qsort(dir_list, dir_count, sizeof(char*), &sort_args);  

        for(int i = 0; i < file_count; i++) {
            struct dirent** dplist;
            char* path = realpath(file_list[i], NULL);

            int numfiles = getDirEntries(file_list[i], &path, &dplist);
            struct stat** dirInfoList = getDirInfo(dplist, path, numfiles);
      
            int is_cwd = (strcmp(path, format_cwd) == 0);
            struct padding_t* pp = malloc(sizeof(struct padding_t)); 
            get_file_padding(pp, file_list, file_count);
     
            if(opt[1]) {
                // -l (long listing)
                outputLongList(opt[0], numfiles, dplist, dirInfoList, path, is_cwd, pp);
            } else {
                outputRegList(opt[0], numfiles, dplist, dirInfoList, path, is_cwd, pp);
            }

            free(path);

            for(int i = 0; i < numfiles; i++) {
                free(dplist[i]);
            }
            free(dplist);

            for(int i = 0; i < numfiles; i++) {
                free(dirInfoList[i]);
            }
    
            free(dirInfoList);
            free(pp); 
        }
        
        if(file_count > 0 && dir_count > 0) {
            printf("\n");
        }

        for(int i = 0; i < dir_count; i++) {
            // ls on directories
            if(opt[2]) {
                recursive_arg(dir_list[i], 1);
            } else {
                struct dirent** dplist; 
                char* file = add_quotes(dir_list[i]); 
                char* path = realpath(dir_list[i], NULL);

                int numfiles = getDirEntries(dir_list[i], &path, &dplist);
                struct stat** dirInfoList = getDirInfo(dplist, path, numfiles); 

                struct padding_t* pp = malloc(sizeof(struct padding_t));
                get_dir_padding(pp, numfiles, dirInfoList);

                if(i != 0) {
                    printf("\n");
                }
                if(argc - idx > 1) {
                    printf("%s:\n", file);
                }

                if(opt[1]) {
                    // -l (long listing)
                    outputLongList(opt[0], numfiles, dplist, dirInfoList, path, 1, pp);
                } else { 
                    outputRegList(opt[0], numfiles, dplist, dirInfoList, path, 1, pp); 
                }

                free(file); 
                free(path); 
                free(pp);

                for(int i = 0; i < numfiles; i++) {
                    free(dplist[i]);
                }
                free(dplist);

                for(int i = 0; i < numfiles; i++) {
                    free(dirInfoList[i]);
                }
                free(dirInfoList);
            }
        }

        for(int i = 0; i < file_count; i++) {
            free(file_list[i]); 
        }
        free(file_list);

        for(int i = 0; i < dir_count; i++) {
            free(dir_list[i]); 
        }
        free(dir_list);

    }

    free(cwd);
    free(format_cwd); 

    return 0; 
}
