#ifndef BUFFER_MNG
#define BUFFER_MNG

#include <math.h>
#include <list>
#include <iostream>
#include <iomanip>
#include <string.h>
#include "catalog.h"
#include "buffer.h"
#include "file.h"
#include "debug.h"

#define DEFAULT_BUFF_LIMITS 3

class Buffer_Manager
{
public:
	Buffer_Manager();
	Buffer_Manager(const short size);
	~Buffer_Manager();
	
	char* get_buf_block(char* table, const int block_num);
	int write2buf(char * table, int * values, int block_num, int mode); 
	void flush_buffer();
	void flush_table(char * table);
	int commit(Catalog *);

	void print_buffer();	
	void print_hit_rate();

private:
	std::list<Buffer> buf_pool;
	int buff_limits;
	int buff_in_use;
	int hits;
	int misses;
};
#endif
