#include "buffer.h"


Buffer::Buffer(unsigned short id)
:block_num(-1),buff_id(id)
{
	strcpy(table_name, "\0");
	//buff_ptr = new char[BLOCK_SIZE];
	buff_ptr=fill();
}

Buffer::Buffer(unsigned int block_size, unsigned short id)
:block_num(-1), buff_id(id)
{
	strcpy(table_name, "\0");
	buff_ptr = new char[block_size];
}

Buffer::~Buffer()
{
	//std::cout<<"call buffer destructor\n";
	delete [] buff_ptr;
}

void Buffer::set_table_name(const char * tn)
{
	strcpy(table_name, tn);	
}
char* Buffer::fill()
{
	int* dat=new int[12];
	for(int i=0;i<12;i++){
		dat[i]=0xC3C3C3C3;
	}
	return (char*)dat;
}
