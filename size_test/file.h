#ifndef FILE_MNG
#define FILE_MNG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>

#define DATA "data/"
#define BLOCK_SIZE 48

/**
 * This is the storage manager 
**/
class File
{
public:
	File();
	File(char * f_name, const char *mode);
	File(const char * table_name, const char* attr_name, const char*mode);
	~File();

	FILE* get_block(const int block_num);
	int disk2buff(FILE * fp, char * buf_ptr);
	int buff2disk(FILE * fp, char * buf_ptr);

	int disk2buff_(const int, char*);
	int buff2disk_(const int, char*);

	int dele_file(char * tab_name);
	int get_file_size();

	void direct_print(int num_attrs);
		
	char* get_path(char * f_name);
	char* get_path(const char*,const char*);

	FILE * fptr;
};
#endif
