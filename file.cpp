#include "file.h"

File::File()
:fptr(NULL)
{}

File::File(char * f_name, const char *mode)
{
	char *path=get_path(f_name);
	fptr=fopen(path, mode);
	if(fptr==NULL)
		fptr=fopen(path,"w");
	free(path);
}

File::File(const char * table_name, const char * attr_name,const char *mode)
{
	char *path=get_path_(table_name, attr_name);
	fptr = fopen(path, mode);
	if(fptr==NULL){
		fptr=fopen(path,"w+");
	}
	free(path);
}

File::~File()
{
	//std::cout<<"File desctructor, closing the file!\n";
	if(fptr!=NULL)
		fclose(fptr);
}
void File::create_file(char* name)
{
	char* path=get_path(name);
	fptr=fopen(path,"w");
	free(path);
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
int File::buff2disk(FILE* fp, char * buf_ptr)
{
	if(fwrite(buf_ptr,BLOCK_SIZE,1,fp)!=1)
		return 0;
	else
		return 1;	
}
int File::buff2disk_apd(char *buf_ptr)
{
	if(fwrite(buf_ptr,BLOCK_SIZE,1,fptr)!=1)
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
	int buf[12];
	while(fread(buf,BLOCK_SIZE,1,fptr)==1){
		for(int i=0;i<num_attrs;i++){
			cout<<setw(13)<<left<<buf[i];	
		}
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

char* File::get_path_(const char * table,const char* attr)
{
	char suffix[]=".index";
	int len=strlen(table)+strlen(attr)+strlen(suffix)+strlen(DATA);
	char *path = (char*)calloc(1,len+2);
	strcat(path,DATA);
	strcat(path, table);
	strcat(path,"_");
	strcat(path, attr);
	strcat(path, suffix);
	path[len+1]='\0';
	//printf("open file: %s\n",path);
	return path;
}

int File::dele_file(const char* table,const char* attr)
{
	char *path=get_path_(table,attr);
	if(remove(path)==0)	
	{
		free(path);
		return 1;
	}else{
		free(path);
		return 0;
	}

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

