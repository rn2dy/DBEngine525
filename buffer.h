#ifndef BUFFER
#define BUFFER

#include <iostream>
#include <string.h>
#define BLOCK_SIZE 48

class Buffer
{
public:
	Buffer(unsigned short buff_id);
	Buffer(unsigned int block_size, unsigned short buff_id);
	~Buffer();

	int get_block_num(){return block_num;}
	void set_block_num(int bn){block_num = bn;} 

	char * get_buff_ptr(){return buff_ptr;}
	void set_buff_ptr(char * ptr){buff_ptr = ptr;}

	char * get_table_name(){return table_name;}
	void set_table_name(const char * tn);

	unsigned short get_buff_id(){return buff_id;}
	char* fill();
	
	unsigned short buff_id;
	int block_num;//block number tells if the it is dirty or not
	char table_name[3];

	char * buff_ptr;//the entry point to the memory 	
};
#endif
