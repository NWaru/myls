#ifndef _UTILITY_H_
#define _UTILITY_H_

char* get_full_path(char* path, char* filename);
char* add_quotes(char* output);
int is_regular_file(char* path);
int is_directory(char* path);

#endif
