#ifndef BPLUS
#define BPLUS
/**
 * B+-Tree, structs and interfaces
**/
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <limits.h>
#include <queue>
#include "file.h"
#include "debug.h"

class bplus_tree
{
public:
	bplus_tree(int debug_mode, const char* table_name, const char* attr_name);
	~bplus_tree();

	int initialize_tree(char dup_mode);
	int insert(int key, int block_number);

	void read_in_header();
	void print_header();
	void update_header();

	int find_it(int key);
	int find(int key, int block_number);
	void get_block(int block,char* buf);

	void print_tree();
	
	struct tree_header &hdr;//this is guaranteed to exist
};

struct tree_header
{
public:
	tree_header();	
	~tree_header(){}	

	int root;
	char duplicate;
	int numOfBlocks;
	int numOfInterNodes;
	int numOfLeafNodes;
	int numOfPtrBucket;
};

class node
{
public:
	node(char t,char s,int p){type=t;size=s;parent_block=p;}
	~node(){}

	//virtual int find(int key)=0;
	//virtual int insert(int key, int table_block_num)=0;
	//virtual int insert_here(int key, int index_block_num)=0;
	//virtual int split()=0;

	char type;
	char size;
	int parent_block;
	int keys[4];
};

class internal_node : public node
{
public:
	internal_node();
	~internal_node();

	int find(int key);
	int insert(int key, int table_block_num);
	int insert_here(int key, int index_block_num);
	int split();

	int pointers[5];
};

class leaf_node : public node
{
public:
	leaf_node();
	~leaf_node();

	int find(int key);
	int insert(int key, int table_block_num);
	int insert_here(int key, int index_block_num);
	int split();
	
	int next_neighbor;
	int pointers[4];
};

class ptr_bucket
{
public:
	ptr_bucket();
	~ptr_bucket();

	int insert(int block_num);
	//int* collect();//return all the block pointers

	int pointers[10];
	int next_bucket;
	short in_use;
};
//some utility functions
void copy_insert_internal(internal_node &i_node, int* keys, int* pointers, int key, int block_num);
void copy_insert_leaf(leaf_node &l_node, int* keys, int* pointers, int key, int block_num);
int insert_in_parent(int right_block,int key,int left_block);
void update_parent(internal_node &i_node, int my_block_num);
void print_bucket(int block_number);
#endif
