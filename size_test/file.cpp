#include "file.h"

File::File()
:fptr(NULL)
{}
File::File(char * f_name, const char *mode)
{
	char *path = get_path(f_name);
	fptr = fopen(path, mode);
	free(path);
}
File::File(const char * table_name, const char * attr_name,const char *mode)
{
	char *path = get_path(table_name, attr_name);
	fptr = fopen(path, mode);
	/*
	if(fptr==NULL)
		printf("Error: file open fail\n");
	*/
	free(path);
}

File::~File()
{
	//std::cout<<"File desctructor, closing the file!\n";
	if(fptr!=NULL)
		fclose(fptr);
}

FILE* File::get_block(const int block_num)
{
	if(fseek(fptr, block_num * BLOCK_SIZE, SEEK_SET)==0)
		return fptr;
	else
		return NULL;
}

int File::disk2buff(FILE* fp, char * buf_ptr)
{
	if(fread(buf_ptr,BLOCK_SIZE,1,fp)!=1)
		return 0;
	else
		return 1;
}

int File::disk2buff_(const int block_num, char * buf_ptr)
{
	if(fseek(fptr, block_num*BLOCK_SIZE, SEEK_SET)==0){
		if(fread(buf_ptr,BLOCK_SIZE,1,fptr)!=1){
			printf("read error\n");
			return 0;
		}
		else	
			return 1;

	}
	printf("seek error\n");
	return 0;
}
int File::buff2disk_(const int block_num, char* buf_ptr)
{
	if(fseek(fptr, block_num*BLOCK_SIZE, SEEK_SET)==0){
		if(fwrite(buf_ptr,BLOCK_SIZE,1,fptr)!=1){
			printf("write error\n");
			return 0;
		}
		else	
			return 1;

	}
	printf("seek error\n");
	return 0;
}
/**
 * Write to the position indicated by block_num
**/
int File::buff2disk(FILE* fp, char * buf_ptr)
{
	if(fwrite(buf_ptr,BLOCK_SIZE,1,fp)!=1)
		return 0;
	else
		return 1;	
}

int File::get_file_size()
{	
	rewind(fptr);
	fseek(fptr, 0L, SEEK_END);
	int size = ftell(fptr);
	rewind(fptr);
	return size;
}

void File::direct_print(int num_attrs)
{
	using namespace std;
	int * buf = new int[12];	
	while(fread(buf,1,BLOCK_SIZE,fptr)==BLOCK_SIZE){
		for(int i=0;i<num_attrs;i++){
			cout<<setw(13)<<left<<*(buf+i);	
		}
		memset(buf,0,BLOCK_SIZE);	
		cout<<"\n";
	}
}
char * File::get_path(char * f_name)
{
	char *path = (char*)malloc(strlen(f_name)+6);
	strcpy(path, DATA);
	strcat(path, f_name);
	return path;
}
char * File::get_path(const char * table,const char* attr)
{
	char suffix[]=".index";
	int len=strlen(table)+strlen(attr)+strlen(suffix);
	char *path = (char*)malloc(len+2);
	strcat(path, table);
	strcat(path,"_");
	strcat(path, attr);
	strcat(path, suffix);
	path[len+1]='\0';
	printf("file path: %s\n",path);
	return path;
}

int File::dele_file(char * f_name)
{
	char *path = get_path(f_name);
	if(remove(path)==0)	
	{
		free(path);
		return 1;
	}else{
		free(path);
		return 0;
	}
}

