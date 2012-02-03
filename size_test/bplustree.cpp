#include "bplustree.h"

// tree_header stores meta data about the b+ tree
tree_header::tree_header()
:root(0),
duplicate('Y'),
numOfBlocks(0),
numOfInterNodes(0),
numOfLeafNodes(0),
numOfPtrBucket(0)
{}

//global variables
File* tfile;
int debug;
//static int debug=0;
tree_header header;

/*********************** bplus_tree ***************************/
bplus_tree::bplus_tree(int mode,const char* table_name,const char* attr_name):hdr(header)
{
	debug=mode;
	tfile=new File(table_name,attr_name,"r+");
}

bplus_tree::~bplus_tree()
{
	update_header();
	delete tfile;
}

int bplus_tree::initialize_tree(char mode)
{
	header.duplicate=mode;
	header.root=0;
	header.numOfBlocks=1;
	header.numOfInterNodes=0;
	header.numOfLeafNodes=0;
	header.numOfPtrBucket=0;
	return tfile->buff2disk_(0, (char*)&header);
}

void bplus_tree::print_header()
{
	std::cout<<"-- Header Info --"<<std::endl;	
	std::cout<<"Root block: "<<header.root<<std::endl;
	std::cout<<"Allow duplicate: "<<header.duplicate<<std::endl;
	std::cout<<"Total number of blocks: "<<header.numOfBlocks<<std::endl;
	std::cout<<"Internal nodes: "<<header.numOfInterNodes<<std::endl;
	std::cout<<"Leaf nodes: "<<header.numOfLeafNodes<<std::endl;
	std::cout<<"pointer bucket: "<<header.numOfPtrBucket<<std::endl;
	std::cout<<std::endl;
}

void bplus_tree::read_in_header()
{
	int res=tfile->disk2buff_(0, (char*)&header);
	//if no header is available, error 
	if(!res){
		printf("Error: read header fail. Header not initialized yet\n");
	}
/*
	if(debug)
		print_header();
*/
}

void bplus_tree::print_tree()
{
if(debug){
	printf("************** This is the whole tree ***************\n");
}	
	using namespace std;
	queue<int> t_que;
	t_que.push(header.root);
	while(!t_que.empty()){
		int block=t_que.front();
		t_que.pop();
		char object[48];
		tfile->disk2buff_(block,object);
		if( ((node*)object)->type=='I'){
			internal_node* i_node=(internal_node*)object;
			if(block==header.root)
				cout<<"Node type: root"<<endl;
			else
				cout<<"Node type: internal"<<endl;
			cout<<"Keys: ";
			for(int i=0;i<4;i++){
				int key=i_node->keys[i];
				if(key==INT_MAX)
					cout<<"nil,";
				else
					cout<<key<<",";
			}
			cout<<"\nPointers: ";
			for(int i=0;i<5;i++){
				int pointer=i_node->pointers[i];
				if(pointer!=0){
					t_que.push(pointer);
					cout<<pointer<<",";
				}
			}
			cout<<endl;
		}
		if(((node*)object)->type=='L'){
			leaf_node* l_node=(leaf_node*)object;
			if(block==header.root)
				cout<<"Node type: root"<<endl;
			else
				cout<<"Node type: leaf"<<endl;
			int l=(int)(l_node->size);
			cout<<"Keys: ";
			for(int i=0;i<l;i++){
				int key=l_node->keys[i];
				if(key==INT_MAX)
					cout<<"nil,";
				else
					cout<<key<<",";
			}
			cout<<"\nPointers: ";
			for(int i=0;i<l;i++){
				cout<<l_node->pointers[i]<<",";
			}
			cout<<endl;
			//print the bucket in duplicate mode
			if(header.duplicate=='Y'){
				for(int i=0;i<4;i++){
					int pointer=l_node->pointers[i];
					if(pointer!=0){
						print_bucket(pointer);
					}
				}	
			}
		}
	}	
}
void bplus_tree::get_block(int block,char*buf)
{
	MSG("Read block");
	tfile->disk2buff_(block,buf);
}
void bplus_tree::update_header()
{
	tfile->buff2disk_(0, (char*)&header);
}

int bplus_tree::insert(int key, int block_number)
{
if(debug){
	printf("insert(key:%d,block_number:%d)\n",key,block_number);
}
	DEBUG_VAR(header.root);
	// if tree is empty
	if(header.root==0){
		/**
		 * Create the root(leaf node)
		 * Create pointer bucket if in duplicate mode, else
		 * Put block_number directly into the pointers array		
		**/
if(debug){
	printf("create the node 1, insert value: %d\n",key);
}
		leaf_node root;
		root.keys[0]=key;	
		root.size++;
		header.root=1;		
		root.parent_block=header.root;
		if(header.duplicate=='N'){
			root.pointers[0]=block_number;
		}else{
			ptr_bucket bucket;	
			bucket.pointers[0]=block_number;
			bucket.in_use++;
			root.pointers[0]=2;
			tfile->buff2disk_(2, (char*)&bucket);
			header.numOfPtrBucket++;
			header.numOfBlocks++;
		}
		tfile->buff2disk_(1, (char*)&root);
		//update header info
		header.numOfBlocks++;
		header.numOfLeafNodes++;
		return 1;
	}//tree is empty 

	//if tree is NOT empty, find the leaf node where to insert 
	int leaf_block=find(key, header.root);
	DEBUG_VAR(leaf_block);

	if(leaf_block==0){
		//this should not happen
		return 0;
	}
	leaf_node l_node;
	tfile->disk2buff_(leaf_block,(char*)&l_node);

	int index=l_node.find(key);
	//
	//  index!=-1, key already exists
	// 	insert fails - if in no-duplicate mode  
	// 	insert into bucket - if in duplicate mode
	//
	if(index!=-1){
		DEBUG_VAR(index);	
		if(header.duplicate=='N'){
			printf("Error: duplicate keys\n");
			return 0;
		}
		//else get the bucket, insert into the bucket 
		ptr_bucket bucket;
		tfile->disk2buff_(l_node.pointers[index], (char*)&bucket);
		bucket.insert(block_number);
		tfile->buff2disk_(l_node.pointers[index], (char*)&bucket);
		return 1;
	}else{
		DEBUG_VAR(index);	
	//
	//  index==-1, key does not exist
	//  	insert into leaf - still have room
	//  	split - no more room
	// 	
		if(l_node.size < 4){
			DEBUG_VAR(l_node.size);	
			l_node.insert(key, block_number);
			tfile->buff2disk_(leaf_block, (char*)&l_node);
			return 1;
		}
if(debug){
	printf("split leaf node...\n");
}
		//else split the node 
		//create a new node
		leaf_node new_leaf;
		int new_block=header.numOfBlocks;
		new_leaf.next_neighbor=l_node.next_neighbor;
		l_node.next_neighbor=new_block;
		// set the same parent block for the split blocks 
		new_leaf.parent_block=l_node.parent_block;
		header.numOfBlocks++;
		header.numOfLeafNodes++;

		//create memories to hold keys and pointers
		int tmp_keys[5];
		int tmp_pointers[5];
		copy_insert_leaf(l_node,tmp_keys,tmp_pointers,key,block_number);
		//fill the old leaf node 
		for(int i=0;i<3;i++){
			l_node.keys[i]=tmp_keys[i];
			l_node.pointers[i]=tmp_pointers[i];
			l_node.size++;
		}
		//fill the new leaf node
		new_leaf.keys[0]=tmp_keys[3];
		new_leaf.pointers[0]=tmp_pointers[3];
		new_leaf.keys[1]=tmp_keys[4];
		new_leaf.pointers[1]=tmp_pointers[4];
		new_leaf.size+=2;

		//write to disk
		tfile->buff2disk_(new_block, (char*)&new_leaf); 
		tfile->buff2disk_(leaf_block, (char*)&l_node);
#ifdef DEBUGX
	
	std::cout<<"tmp_keys: ";
	for(int i=0;i<5;i++)
		std::cout<<tmp_keys[i]<<", ";
	std::cout<<"\ntmp_pointers: ";
	for(int i=0;i<5;i++)
		std::cout<<tmp_pointers[i]<<", ";
	std::cout<<"\n";
#endif		
		//call insert_in_parent
		int res=insert_in_parent(leaf_block,new_leaf.keys[0],new_block);
		return res;
	}//key does not exist
}

int insert_in_parent(int left_block, int key, int right_block)
{
if(debug){
	printf("insert_in_parent(left_block:%d, key:%d, right_block:%d)\n",left_block,key,right_block);
}
	char node_l[48];
	char node_r[48];
	tfile->disk2buff_(left_block, node_l); 	
	tfile->disk2buff_(right_block, node_r);

	// if left_block is the root block
	if(header.root==left_block){
if(debug){
		printf("split root...\n");
}
		MSG_VAR("Root block was",left_block);
		internal_node new_root;
		int new_block=header.numOfBlocks;
		header.root=new_block;
		MSG_VAR("Root block now is",new_block);
		new_root.keys[0]=key;
		new_root.pointers[0]=left_block;
		new_root.pointers[1]=right_block;
		new_root.size++;
		new_root.parent_block=new_block;
		tfile->buff2disk_(new_block, (char*)&new_root);
		header.numOfBlocks++;
		header.numOfInterNodes++;

		((node*)node_l)->parent_block=new_block;
		((node*)node_r)->parent_block=new_block;
#ifdef DEBUGX
	std::cout<<"node_l keys:"<<std::endl;
	for(int i=0;i<4;i++)
		std::cout<<((internal_node*)node_l)->keys[i]<<", ";
	std::cout<<"\nnode_l pointers:"<<std::endl;
	for(int i=0;i<5;i++)
		std::cout<<((internal_node*)node_l)->pointers[i]<<", ";
	std::cout<<std::endl;
	std::cout<<"node_r keys:"<<std::endl;
	for(int i=0;i<4;i++)
		std::cout<<((internal_node*)node_r)->keys[i]<<", ";
	std::cout<<"\nnode_r pointers:"<<std::endl;
	for(int i=0;i<5;i++)
		std::cout<<((internal_node*)node_r)->pointers[i]<<", ";
	std::cout<<std::endl;
#endif

		tfile->buff2disk_(left_block,node_l);
		tfile->buff2disk_(right_block,node_r);
		return 1;
	}
	//else
	//find the parent of left_block, must be an internal node
	int parent=((node*)node_l)->parent_block;	
	DEBUG_VAR(parent);
	internal_node node_p;
	tfile->disk2buff_(parent, (char*)&node_p);
	//
	//if parent node does not need to split
	//
	if(node_p.size<4){
		DEBUG_VAR(node_p.size);
		node_p.insert_here(key, right_block);	
		tfile->buff2disk_(parent, (char*)&node_p);
		return 1;
	}

	//
	//else split and recursion
	//
if(debug){
	printf("split internal node...\n");
}
	int tmp_keys[5];
	int tmp_pointers[6];	
	copy_insert_internal(node_p, tmp_keys, tmp_pointers, key, right_block);	

#ifdef DEBUGX
	std::cout<<"tmp_keys: ";
	for(int i=0;i<5;i++)
		std::cout<<tmp_keys[i]<<", ";
	std::cout<<"\ntmp_pointers: ";
	for(int i=0;i<6;i++)
		std::cout<<tmp_pointers[i]<<", ";
	std::cout<<"\n";
#endif		

	//fill the old node	
	for(int i=0;i<3;i++){
		node_p.keys[i]=tmp_keys[i];
		node_p.pointers[i]=tmp_pointers[i];
		node_p.size++;
	}
	node_p.pointers[3]=tmp_pointers[3];

	//fill the new node of keys, pointers
	internal_node new_node;
	int new_block=header.numOfBlocks;
	new_node.keys[0]=tmp_keys[3];
	new_node.keys[1]=tmp_keys[4];
	new_node.pointers[0]=0;
	new_node.pointers[1]=tmp_pointers[4];
	new_node.pointers[2]=tmp_pointers[5];
	new_node.size+=2;
	new_node.parent_block=node_p.parent_block;

	header.numOfBlocks++;
	header.numOfInterNodes++;
	
	update_parent(new_node,new_block);

#ifdef DEBUGX
	std::cout<<"node_p keys: ";
	for(int i=0;i<4;i++){
		if(node_p.keys[i]==INT_MAX)
			printf("%s, ","nil");
		else
			printf("%d, ",node_p.keys[i]);
	}
	std::cout<<"\nnode_p pointers: ";
	for(int i=0;i<5;i++)
		std::cout<<node_p.pointers[i]<<", ";
	std::cout<<"\n";
#endif	

	tfile->buff2disk_(parent, (char*)&node_p);
	tfile->buff2disk_(new_block, (char*)&new_node);

	return insert_in_parent(parent, tmp_keys[3], new_block); 
}

void copy_insert_internal(internal_node &i_node,int* tmp_keys,int*tmp_pointers,int key,int block_num)
{
#ifdef DEBUGX
	printf("copy_insert_internal(_,_,_,%d,%d)\n",key,block_num);
#endif

	int pst=0;
	for(;pst<4;pst++){
		if(i_node.keys[pst]>key){
			break;
		}
	}
	//copy keys
	for(int i=0;i<pst;i++)
		tmp_keys[i]=i_node.keys[i];
	tmp_keys[pst]=key;
	for(int i=pst;i<4;i++)
		tmp_keys[i+1]=i_node.keys[i];
	//copy pointers
	for(int i=0;i<=pst;i++)
		tmp_pointers[i]=i_node.pointers[i];
	tmp_pointers[pst+1]=block_num;
	for(int i=pst+1;i<5;i++)
		tmp_pointers[i+1]=i_node.pointers[i];
	
	//irease data of i_node 			
	for(int i=0;i<4;i++)
		i_node.keys[i]=INT_MAX;
	for(int i=0;i<5;i++)
		i_node.pointers[i]=0;
	i_node.size=0;
}

void copy_insert_leaf(leaf_node &l_node, int*tmp_keys, int*tmp_pointers, int key, int block_num)
{
#ifdef DEBUGX
	printf("copy_insert_leaf(_,_,_,%d,%d)\n",key,block_num);
#endif
	int pst=0;
	for(;pst<4;pst++){
		if(l_node.keys[pst]>key){
			break;
		}
	}
	//copy
	for(int i=0;i<pst;i++){
		tmp_keys[i]=l_node.keys[i];
		tmp_pointers[i]=l_node.pointers[i];
	}	

	for(int i=pst;i<4;i++){
		tmp_keys[i+1]=l_node.keys[i];
		tmp_pointers[i+1]=l_node.pointers[i];
	}	
	tmp_keys[pst]=key;
	//if allow duplicate
	if(header.duplicate=='Y'){
		ptr_bucket bucket;
		int new_block=header.numOfBlocks;
		bucket.pointers[0]=block_num;		
		bucket.in_use++;
		tfile->buff2disk_(new_block, (char*)&bucket);
		tmp_pointers[pst]=new_block;
		header.numOfBlocks++;
		header.numOfPtrBucket++;
	}else{
		tmp_pointers[pst]=block_num;
	}

	//irease data from l_node
	for(int i=0;i<4;i++)
		l_node.keys[i]=INT_MAX;
	for(int i=0;i<4;i++)
		l_node.pointers[i]=0;
	l_node.size=0;
}
void update_parent(internal_node &i_node, int my_block_num){
	for(int i=0; i<5; i++){
		if(i_node.pointers[i]!=0){
			char object[48];	
			tfile->disk2buff_(i_node.pointers[i],object);
			((node*)object)->parent_block=my_block_num;
			tfile->buff2disk_(i_node.pointers[i],object);
		}	
	}
}
/**
 * delegates to find() to make life easier
 * return 0 - the key does not exist
 * return data pointer - otherwise
**/
int bplus_tree::find_it(int key)
{
if(debug){
	printf("bplus_tree find(key:%d)\n",key);
}
	//if tree is empty
	if(header.root==0)
		return -2;//return header, root==null 
	//else
	int leaf_block=find(key, header.root);		
	MSG_VAR("leaf_block", leaf_block);

	if(leaf_block==0){
		//this should not happen
		MSG("panic");
		return 0;
	}
	//else
	leaf_node l_node;
	tfile->disk2buff_(leaf_block, (char*)&l_node); 
	
if(debug){
	printf("Found Leaf node: \n");
	printf("keys: ");
	for(int i=0;i<4;i++){
		if(l_node.keys[i]==INT_MAX)
			printf("%s, ","nil");
		else
			printf("%d, ",l_node.keys[i]);
	}
	printf("\npointers: ");
	for(int i=0;i<4;i++)
		printf("%d, ",l_node.pointers[i]);	
	printf("\n");	
}
	DEBUG_VAR(l_node.size);
	DEBUG_VAR(l_node.parent_block);
	DEBUG_VAR(l_node.next_neighbor);
	int index=l_node.find(key);
	if(index==-1){
		MSG_VAR("Key not in the leaf node",key);
		return index;
	}else{
		MSG_VAR("Found in the leaf node",key);
if(debug){
	if(header.duplicate=='Y')
		print_bucket(l_node.pointers[index]);
}
		return l_node.pointers[index];	
	}
	//else return data pointer/data
}
void print_bucket(int block_number)
{
	ptr_bucket bucket;
	std::cout<<"Node type: bucket"<<std::endl;
	std::cout<<"Pointers: ";
	do{
		tfile->disk2buff_(block_number, (char*)&bucket);
		for(int i=0;i<bucket.in_use;i++)
			std::cout<<bucket.pointers[i]<<",";
		block_number=bucket.next_bucket;
	}while(block_number!=0);
	std::cout<<std::endl;
}
// find method - find the leaf block where the key should go 
int bplus_tree::find(int key, int block_number) {
if(debug){
	printf("find(key:%d,block_number:%d)\n",key,block_number);
}
	// block_number is null, how?
	if(block_number==0){
		return block_number;
	}

	//else leaf block is not null 
	char object[48];	
	//read block data to object 
	tfile->disk2buff_(block_number, object); 	
	//find differently according to the node type
	
	if( ((node*)object)->type=='I'){
		internal_node* i_node=(internal_node*)object;	

if(debug){
	printf("Found Internal node: \n");
	printf("Keys: ");
	for(int i=0;i<4;i++){
		if(i_node->keys[i]==INT_MAX)
			printf("%s, ","nil");
		else
			printf("%d, ",i_node->keys[i]);
	}
	printf("\nPointers: ");
	for(int i=0;i<5;i++)
		printf("%d, ",i_node->pointers[i]);	
	printf("\n");	
}
		DEBUG_VAR(i_node->parent_block);
		//find the key directly, using function call won't work!
		int i=0;
		for(;i<4;i++){
			if(i_node->keys[i]>key){
				break;
			}
		}
		MSG_VAR("i",i);
		return find(key, (i_node->pointers)[i]);
	}else if( ((node*)object)->type=='L'){
		return block_number;
	}else{
		MSG("Error");
		return 0;
	}
}

/********************** INTERNAL NODE*************************/
internal_node::internal_node()
:node('I',0,0)
{
	for(int i=0;i<4;i++)
		keys[i]=INT_MAX;
	for(int i=0;i<5;i++)
		pointers[i]=0;
}
internal_node::~internal_node()
{}

int internal_node::find(int key)
{
if(debug){
	printf("internal node: find value: %d\n",key);
}
	int i=0;
	for(;i<4;i++){
		if(keys[i]>key) 
			return i;
	}
	return i;
}

int internal_node::insert(int key,int table_block_num)
{
	//not used
	return 0;
}

int internal_node::insert_here(int key,int index_block_num)
{
if(debug){
	printf("internal node: insert here...\n");
}
	int pos=-1;
	for(int i=0;i<4;i++){
		if(keys[i]>key){
			pos=i;	
			break;
		}	
	}
	//shift the keys/pointers
	for(int i=3;i>pos;i--){
		keys[i]=keys[i-1];
		pointers[i+1]=pointers[i];
	}
	keys[pos]=key;
	pointers[pos+1]=index_block_num;
#ifdef DEBUGX
	std::cout<<"node_p keys: ";
	for(int i=0;i<4;i++){
		if(keys[i]==INT_MAX)
			printf("%s, ","nil");
		else
			printf("%d, ",keys[i]);
	}
	std::cout<<"\nnode_ p pointers: ";
	for(int i=0;i<5;i++)
		std::cout<<pointers[i]<<", ";
	std::cout<<"\n";
#endif
	size++;
	return 1;
}

int internal_node::split()
{
	return 0;
}

/*************************LEAF NODE ********************************/
leaf_node::leaf_node()
:node('L',0,0),next_neighbor(0)
{
	for(int i=0;i<4;i++)
		keys[i]=INT_MAX;
	for(int i=0;i<4;i++)
		pointers[i]=0;
}
leaf_node::~leaf_node(){}

int leaf_node::find(int key)
{
if(debug){
	printf("leaf node: find value: %d\n",key);
}
	int len=(int)size;
	MSG_VAR("leaf key size", len);
	for(int i=0;i<len;i++){
		if(keys[i]==key) return i;
	}
	return -1;//key doesn't exist
}

//this method is called when key does not exist and keys is not full 
int leaf_node::insert(int key, int table_block_num)
{
if(debug){
	printf("leaf node: insert here...\n");
}
	// Find the smallest value that is bigger than the key
	// Insert after it
	int pst=-1;
	//DEBUG_VAR(len);
	for(int i=0;i<4;i++){
		if(keys[i]>key){
			DEBUG_VAR(keys[i]);
			DEBUG_VAR(i);
			pst=i;
			break;
		}
	}
	//swaping	
	for(int i=3;i>pst;i--){
		keys[i]=keys[i-1];
		pointers[i]=pointers[i-1];
	}
	keys[pst]=key;
	//if allow duplicate
	if(header.duplicate=='Y'){
		ptr_bucket bucket;
		int new_block=header.numOfBlocks;
		bucket.pointers[0]=table_block_num;		
		bucket.in_use++;
		tfile->buff2disk_(new_block, (char*)&bucket);
		pointers[pst]=new_block;
		header.numOfBlocks++;
		header.numOfPtrBucket++;
	}else{
		pointers[pst]=table_block_num;
	}
	size++;	
	return 1;
}

int leaf_node::insert_here(int key, int index_block_num)
{
	return 0;
}

int leaf_node::split()
{
	return 0;
}

/*********************** Pointer Bucket *********************/
ptr_bucket::ptr_bucket()
:in_use(0),next_bucket(0)
{
	for(int i=0;i<10;i++)
		pointers[i]=0;
}

ptr_bucket::~ptr_bucket()
{}

int ptr_bucket::insert(int block_number)
{
if(debug){
	printf("bucket: insert here...\n");
}
	//if the block number exists already, do nothing
	for(int i=0;i<in_use;i++){
		if(block_number==pointers[i])
			return 1;
	}
	int next=next_bucket;
	while(next){
		ptr_bucket tmp;
		tfile->disk2buff_(next,(char*)&tmp);
		for(int i=0;i<tmp.in_use;i++){
			if(block_number==tmp.pointers[i])
				return 1;
		}
		next=tmp.next_bucket;
	}
				
	if(in_use<10){
		pointers[in_use]=block_number;	
		in_use++;
	}else if(next_bucket==0){
		ptr_bucket new_bucket;	
		int new_block=header.numOfBlocks;
		this->next_bucket=new_block;
		new_bucket.pointers[0]=block_number;
		new_bucket.in_use++;
		tfile->buff2disk_(new_block, (char*)&new_bucket);
		header.numOfBlocks++;
		header.numOfPtrBucket++;
	}else{
		ptr_bucket new_bucket;
		tfile->disk2buff_(next_bucket, (char*)&new_bucket);
		new_bucket.insert(block_number);
		tfile->buff2disk_(next_bucket, (char*)&new_bucket);
	}
	return 1;
}
