#include "buffer_mng.h"


Buffer_Manager::Buffer_Manager()
:hits(0),misses(0),buff_in_use(0),buff_limits(DEFAULT_BUFF_LIMITS)
{}
Buffer_Manager::Buffer_Manager(const short size)
:hits(0),misses(0),buff_in_use(0),buff_limits(size)
{}

Buffer_Manager::~Buffer_Manager()
{}

char * Buffer_Manager::get_buf_block(char* table_name, const int block_num)
{
	std::list<Buffer>::iterator it=buf_pool.begin();
	for(;it!=buf_pool.end();++it)
	{
		int res=strcmp(it->table_name, table_name);
		if(res==0&&it->block_num==block_num){
			hits++;
			//DEBUG_VAR(hits);
			Buffer* tmp=new Buffer(it->buff_id);
			tmp->set_table_name(it->table_name);
			tmp->block_num=it->block_num;
			memcpy(tmp->buff_ptr, it->buff_ptr, BLOCK_SIZE);
			buf_pool.erase(it);	
			buf_pool.push_back(*tmp);
			return tmp->buff_ptr;
		}
	}

	/**
	 * Not finding the block
	 * Buffer pool is not full -> load a new buffer and read from disk 
	**/
	misses++;
	if(buff_in_use<buff_limits){
		//DEBUG_VAR(buff_in_use);
		//DEBUG_VAR(misses);
		Buffer* tmp=new Buffer(++buff_in_use);
		tmp->set_table_name(table_name);
		tmp->block_num=block_num;
		{
			File file(table_name, "r+");
			file.disk2buff_(block_num,tmp->buff_ptr);
		}
		buf_pool.push_back(*tmp);
		return tmp->buff_ptr;
	}
	/** 
	 * Buffer pool is full -> evict the LRU buffer
	 * and read the required block from disk
	**/
	//DEBUG_VAR(misses);
	it=buf_pool.begin();
	{
		//write LRU buffer's content to disk first
		File file(it->table_name,"r+");	
		file.buff2disk_(it->block_num, it->buff_ptr);
	}
	//open the file and get the block
	Buffer* tmp=new Buffer(it->buff_id);
	tmp->set_table_name(table_name);
	tmp->block_num=block_num;
	{
		File file(table_name,"r");
		file.disk2buff_(block_num, tmp->buff_ptr);
	}
	buf_pool.erase(it);
	buf_pool.push_back(*tmp);
	return tmp->buff_ptr;		
}	
		
int Buffer_Manager::write2buf(char *table,int* dat,int size,int insertPos)
{
	// calculate useful matric
	int block_num=floor(insertPos/48);
	int inBufSize=insertPos%48;
	if(buff_in_use==0){
		//DEBUG_VAR(buff_in_use);
		Buffer* ptr = new Buffer(++buff_in_use);
		ptr->set_table_name(table);
		if(insertPos!=0){
			misses++;
			//DEBUG_VAR(misses);
			File file(table,"r+");
			file.disk2buff_(block_num,ptr->buff_ptr);
			memcpy(ptr->buff_ptr+inBufSize,dat,size);
		}else{
			hits++;
			//DEBUG_VAR(hits);
			memcpy(ptr->buff_ptr,dat,size);
		}
		ptr->block_num=block_num;
		buf_pool.push_back(*ptr);
		return block_num;
	}
	//find a spot to put this rec in
	std::list<Buffer>::iterator it=buf_pool.begin();
	for(;it!=buf_pool.end();it++){
		//DEBUG_VAR(it->table_name);
		if(strcmp(it->table_name,table)==0){
			//MSG("Match table name!");
			if(it->block_num==block_num){
				//MSG("Match block number!");
				hits++;
				memcpy(it->buff_ptr+inBufSize,dat,size);
				return it->block_num;
			}
		}
	}
	//no such spot, check if buffer pool is full or not
	if(buff_in_use<buff_limits){
		//DEBUG_VAR(buff_in_use);
		Buffer* ptr=new Buffer(++buff_in_use);		
		ptr->set_table_name(table);	
		// inBufSize==0 means needs a new block
		if(inBufSize==0){
			hits++;
			//DEBUG_VAR(hits);	
			memcpy(ptr->buff_ptr,dat,size);	
		}else{
			misses++;
			//DEBUG_VAR(misses);	
			File file(table,"r+");	
			file.disk2buff_(block_num,ptr->buff_ptr);
			memcpy(ptr->buff_ptr+inBufSize,dat,size);
		}	
		ptr->block_num=block_num;
		buf_pool.push_back(*ptr);
		return block_num;
	}
	//DEBUG_VAR(buff_in_use);
	// buffer pool is full, evict the LRU block	
	it=buf_pool.begin();
	{
		File file(it->table_name,"r+");
		file.buff2disk_(it->block_num,it->buff_ptr);
	}
	Buffer* ptr=new Buffer(it->buff_id);
	ptr->set_table_name(table);
	if(inBufSize==0){
		hits++;
		//DEBUG_VAR(hits);
		memcpy(ptr->buff_ptr,dat,size);
	}else{
		misses++;
		//DEBUG_VAR(misses);
		File file(table,"r+");
		file.disk2buff_(block_num, ptr->buff_ptr);
		memcpy(ptr->buff_ptr+inBufSize, dat, size);
	}
	ptr->block_num=block_num;
	buf_pool.erase(it);
	buf_pool.push_back(*ptr);
	return block_num;
/*	
	int i = MAX_BUFFS;
	while(i){
		//find if there is empty buffer
		if(it->block_num==-1)
			break;
		i--;
		it++;
	}
	if(it->block_num != -1){//no empty buffer
		//printf("F.1.2\n");
		it=buf_pool.begin();
	}	
	Buffer* tmp=new Buffer(it->buff_id);
	tmp->block_num=block_num;
	tmp->set_table_name(table);
	memcpy(tmp->buff_ptr, dat, BLOCK_SIZE);
	buf_pool.erase(it);
	buf_pool.push_back(*tmp);
	// auto commit
	if(opt){
		File file(table,"a"); 			
		return file.buff2disk_(block_num,(char*)dat);
	}
	return 1;
*/
}

int Buffer_Manager::commit(Catalog *cat)
{
	std::list<Buffer>::iterator it=buf_pool.begin();
	for(;it!=buf_pool.end();++it){
		if(cat->search_table(it->table_name)==0){
			continue;
		}else{
			File file = File(it->table_name, "r+");	
			file.buff2disk_(it->block_num, it->buff_ptr);
		}
	}
	return 1;
}

void Buffer_Manager::flush_table(char * table)
{
	std::list<Buffer>::iterator it;
	for(it = buf_pool.begin(); it != buf_pool.end(); ++it)
	{
		if(strcmp(it->table_name,table)==0){
			it->block_num=-1;
			it->set_table_name("");	
			memset(it->buff_ptr,0,BLOCK_SIZE);
		}
	}
}

void Buffer_Manager::flush_buffer()
{
	std::list<Buffer>::iterator it;
	for(it = buf_pool.begin(); it != buf_pool.end(); ++it)
	{
		it->block_num=-1;
		it->set_table_name("");	
		memset(it->buff_ptr, 0, BLOCK_SIZE);
	}
}

void Buffer_Manager::print_buffer()
{
	using namespace std;
	cout << "Buffer#   Table   Block#   Data" << endl; 
	cout << "-------   -----   ------   ---- LRU" << endl;
	list<Buffer>::iterator it;
	for(it=buf_pool.begin(); it != buf_pool.end(); ++it)	
	{
		cout <<setw(10)<<left<<it->buff_id;
		cout <<setw(8)<<left<<it->table_name;
		cout <<setw(9)<<left<<it->block_num;
		int * buf = (int*)(it->buff_ptr);
		for(int i = 0; i < 12; i++)		
		{
			int res=buf[i];
			if(res!=0xC3C3C3C3)
				cout<<res<<" ";	
		}
		cout <<"\n";
	}
	cout << "-------   -----   ------   ---- MRU" << endl;
}

void Buffer_Manager::print_hit_rate()
{
	std::cout<<"Buffer hit rate is: " <<std::endl;
	std::cout<<"Hits: "<<hits;
	std::cout<<"  Misses: "<<misses;
	std::cout << "  Total: " <<(hits+misses)<< std::endl;
}
