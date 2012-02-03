#include "query.h"
int debug=0;
void dispatch_query(statement_t * st)
{
	if(st->set){
		if((st->set)->variable==CONFIG_INDEX_DEBUG)
			debug=st->set->value;
	}
	if(st->create_table){
		create_table(st->create_table);
	}	
	if(st->insert){
		insert(st->insert);
	}
	if(st->select){
		select(st->select);
	}
	if(st->drop_table){
		drop_table(st->drop_table);
	}
	if(st->print_table){
		print_table_fast(st->print_table);
	}
	if(st->create_index){
		create_index(st->create_index);
	}
	if(st->drop_index){
		drop_index(st->drop_index);
	}
	if(st->print_index){
		print_index(st->print_index);
	}
	if(st->parameterless == CMD_PRINT_CATALOG){
		cat->print_catalog();	
	}
	if(st->parameterless == CMD_PRINT_BUFFER){
		buf_mng->print_buffer();	
	} 
	if(st->parameterless == CMD_PRINT_HIT_RATE){
		buf_mng->print_hit_rate();
	}
	if(st->parameterless == CMD_COMMIT){
		printf("Commit...\n");
		buf_mng->commit(cat);
	}
	if(st->parameterless == CMD_EXIT){
		printf("Commit...\n");
		buf_mng->commit(cat);
	}
}

int create_table(create_table_statement_t * st)
{
	MSG("create table...");
	if( length_id_list(st->columns) > 8 ){
		printf("Num of columns more than 8!\n");
		return 0;
	}
	return cat->add_rec(st->table, st->columns);
}

int insert(insert_statement_t * st)
{
	MSG("insert....");
	if(cat->search_table(st->table))	
	{
		//verify num of args		
		int in_num=length_num_list(st->values);
		int real_num=cat->get_num_attrs(st->table);
		if(in_num == real_num)
		{
			int * dat=produce_dat(st->values, in_num);
			Table &tb=cat->tables[st->table];
			int oldInsertPos=tb.insertPos;
			int oldNumOfRec=tb.numOfRec;

		/******************** update index *************************/
			int insertPos=cat->add_rec_count(st->table); 
			int block_num=floor(insertPos/48);
			int numOfIndex=tb.numOfIndex;
			if(numOfIndex!=0){
				int *idx_pos=cat->get_index_attr(st->table,numOfIndex);
				// Check if duplicate key being accidentally 
				// inserted into the file
				if(numOfIndex>1){
					for(int i=0;i<numOfIndex;i++){
						const char* attr=tb.attributes[idx_pos[i]].c_str();
						bplus_tree bpt(debug,st->table,attr);
						bpt.read_in_header();
						if(bpt.hdr.duplicate=='N'){
							int res=bpt.find_it(dat[idx_pos[i]]);
							if(res!=-2&&res!=-1){
								printf("Error:duplicate keys detected\n");	
								tb.insertPos=oldInsertPos;
								tb.numOfRec=oldNumOfRec;
								delete [] dat;
								delete [] idx_pos;
								return 0;
							}
						}
					}
				}
				for(int i=0;i<numOfIndex;i++){
					const char* attr_name=tb.attributes[idx_pos[i]].c_str();
					DEBUG_VAR(attr_name);
					DEBUG_VAR(dat[idx_pos[i]]);
					bplus_tree bpt(debug,st->table,attr_name);	
					bpt.read_in_header();
					int ret=bpt.insert(dat[idx_pos[i]],block_num); 
					if(ret==0){
						tb.insertPos=oldInsertPos;
						tb.numOfRec=oldNumOfRec;
						delete [] dat;
						delete [] idx_pos;
						return 0;
					}
				}
				delete [] idx_pos;
			}
/********************** insert into buffer now *********************/

			int size=in_num*sizeof(int);
			DEBUG_VAR(insertPos);
			DEBUG_VAR(size);
			buf_mng->write2buf(
					st->table,
					dat,
					size,
					insertPos);
			delete [] dat;
		}else{
			printf("Incorrect num of insert values!\n");
			return 0;
		}
	}else{
		printf("No such table!\n");
		return 0;
	}
}

int select(select_statement_t * st)
{
	if(cat->search_table(st->table)){
		if((cat->tables[st->table]).numOfRec==0){
			printf("Empty table\n");
			return 0;
		}
		if(st->fields==NULL){
			//select * from table x /where...
			if(st->conditions==NULL)
				return print_table1(st);	
			else
				return print_table3(st,NULL);
		}else{
			//select ... from table x /where...	 
			int loc[length_id_list(st->fields)];
			if(cat->search_attr2(st->table, st->fields, loc)==0){
				printf("Incorrect attributes\n");
				return 0;
			}
			if(st->conditions==NULL)
				return print_table2(st, loc);
			else
				return print_table3(st, loc);
		}
	}else{
		printf("No such table!\n");
		return 0;
	}
}

/*********************** INDEX FUNCTION ******************************/
int create_index(create_index_statement_t* st)
{
	MSG("create index");
	// check CATALOG if table exists
	if(!cat->search_table(st->table)){
		printf("No such table %s\n",st->table);
		return 0;
	}
	if(cat->search_attr1(st->table,st->column)==-1){
		printf("No such attribute in table %s\n",st->table);
		return 0;
	}
	// check CATALOG if index has already been created or name been used
	int ret=cat->cat_check_index(st->table,st->index,st->column);
	if(ret==-1){
		printf("Index name %s is already been used\n",st->index);
		return 0;
	}
	if(ret==-2){
		printf("Index on %s has already been created\n",st->column);
		return 0;
	}
	// else
	if(!cat->cat_create_index(st->table,st->index,st->column)){
		printf("Index is full in table %s\n",st->table);
		return 0;
	}
	// now, create the actual index file
	bplus_tree bpt(debug, st->table, st->column);
	if(st->duplicates)
		bpt.initialize_tree('Y');	
	else
		bpt.initialize_tree('N');
	//get location of that attributes, get how many blocks
	Table &tb=cat->tables[st->table];
	int loc=tb.getAttrsIndex(st->column);
	int blocks=ceil(tb.insertPos/48.0);
	int rowWidth=tb.numOfAttrs;
	int recsInBlock=12/rowWidth;
	int fail=0;
	for(int i=0;i<blocks&&!fail;i++){
		int* dat=(int*)buf_mng->get_buf_block(st->table,i);	
		int datChunk[rowWidth];	
		int offset=0;
		for(int j=0;j<recsInBlock;j++){
			memcpy(datChunk,dat+offset,rowWidth*sizeof(int));
			if(datChunk[0]==0xC3C3C3C3){
				break;
			}
			int key=datChunk[loc];
			DEBUG_VAR(key);
			DEBUG_VAR(i);
			if(!bpt.insert(key,i)){
				drop_index(st->table,st->index);
				printf("Index creation fail\n");
				return 0;
			}
			offset+=rowWidth;
		}
	}
	return 1;
}
	
int drop_index(index_ref_t* st){
	return drop_index(st->table,st->index);
}
int drop_index(const char*table,const char*index)
{
	MSG("Drop index");
	// check CATALOG if table exists
	if(!cat->search_table(table)){
		printf("No such table!");
		return 0;
	}
	// check CATALOG if index could be droped
	if(cat->cat_check_index(table,index,NULL)==-1){
		Table &tb=cat->tables[table];
		File file;
		file.dele_file(table,tb.index_name[index].c_str());
		cat->cat_drop_index(table,index);
		return 1;
	}
	// else
	printf("No such index to be dropped\n");
	return 0;
}

int print_index(index_ref_t* st)
{
	MSG("Print index");
	// check CATALOG if table exists
	if(!cat->search_table(st->table)){
		printf("No such table!\n");
		return 0;
	}
	// check CATALOG if the index exist 
	if(cat->cat_check_index(st->table, st->index, NULL)==-1){
		Table &tb=cat->tables[st->table];
		bplus_tree bpt(debug, st->table, tb.index_name[st->index].c_str());
		bpt.read_in_header();
		bpt.print_tree();
		return 1;
	}
	printf("no such index\n");	
	return 0;
}

/************************ Print functions ************************/
int print_table1(select_statement_t * st)
{
	MSG("Print_table1...");	
	using namespace std;
	Table &table = cat->tables[st->table];	
	int totl_bl=ceil(table.insertPos/48.0);
	int rowWidth=table.numOfAttrs;
	for(int i=0;i<rowWidth;i++){	
		cout<<setw(13)<<left<<table.attributes[i];
	}	
	cout<<"\n";
	for(int i=0;i<totl_bl;i++){
		int* dat = (int *)buf_mng->get_buf_block(st->table, i);
		if(dat==NULL){
			return 0;
		}
		for(int j=0;j<12; j++){
			if(dat[j]==0xC3C3C3C3)
				break;
			cout<<setw(13)<<left<<dat[j];
			if((j+1)%rowWidth==0)
				cout<<"\n";
		}
	}
	return 1;
}

int print_table2(select_statement_t * st, int * loc)
{
	MSG("Print_table2...");	
	using namespace std;
	Table &table=cat->tables[st->table];
	int totl_bl=ceil(table.insertPos/48.0);
	int rowWidth=table.numOfAttrs;
	id_list_t* id_lst=st->fields;
	while(id_lst){
		cout<<setw(13)<<left<<id_lst->id;
		id_lst=id_lst->next;
	}
	cout<<"\n";
	int len=length_id_list(st->fields);
	int rounds=12/rowWidth;
	DEBUG_VAR(rounds);
	for(int i=0;i<totl_bl;i++){
		int* dat = (int *)buf_mng->get_buf_block(st->table,i);
		if(dat==NULL)
			return 0;
		int offset=0;
		int k=rounds;
		int done=0;
		while(k>0&&!done){
			for(int j=0;j<len;j++){
				int num=dat[loc[j]+offset];
				if(num==0xC3C3C3C3){
					done=1;
					break;
				}
				cout<<setw(13)<<left<<num;
			}	
			offset+=rowWidth;
			k--;
			cout<<"\n";
		}
	}
	return 1;
}
int check_condition(select_statement_t* st)
{
	condition_t* cur=st->conditions;
	do{
		int ret=cat->search_attr1(st->table,cur->left_col);
		if(cur->right_col){
			ret=cat->search_attr1(st->table,cur->right_col);
		}
		if(ret==-1)	
			return 0;	
		cur=cur->next;
	}while(cur);
	return 1;
}
int print_table3(select_statement_t * st, int * loc)
{
	MSG("Print_table3...");	
	if(!check_condition(st)){
		printf("where clause wrong\n");
		return 0;
	}
	//check which col is indexed, always select the first one if all are indexed
	condition_t* ct=st->conditions;	
	do{
		if(cat->cat_check_index_col(st->table,ct->left_col)) break;
		if(ct->next==NULL) break;
		ct=ct->next;
	}while(ct);
	//printf("left_col:%s\n",ct->left_col);	
// print the column names, and calculate some parameter
	using namespace std;
	Table &table=cat->tables[st->table];	
	int totl_bl=ceil(table.insertPos/48.0);
	int rowWidth=table.numOfAttrs;
	int recsInBlock=12/rowWidth;
	DEBUG_VAR(totl_bl);
	DEBUG_VAR(rowWidth);
	DEBUG_VAR(recsInBlock);
	int len=length_id_list(st->fields);
	if(loc==NULL){
		for(int i=0;i<rowWidth;i++){	
			cout<<setw(13)<<left<<table.attributes[i];
		}
	}else{
		id_list_t * id_lst = st->fields;	
		while(id_lst){
			cout<<setw(13)<<left<<id_lst->id;
			id_lst = id_lst -> next;
		}
	}
	cout<<"\n";
// print the column values
	if(cat->cat_check_index_col(st->table, ct->left_col)){// check again, redundant!
		bplus_tree bpt(debug,st->table,ct->left_col);
		bpt.read_in_header();
		if(ct->op==OP_EQUAL){
			//right hand need to be a number
			if(!ct->right_col){
				int block=bpt.find_it(ct->right_num);
				if(block==-1){
					printf("No such records!\n");
					return 0;	
				}
				if(bpt.hdr.duplicate=='Y'){
					ptr_bucket bucket;
					int next=block;
					DEBUG_VAR(next);
					while(next){
						bpt.get_block(next,(char*)&bucket);
						for(int i=0;i<bucket.in_use;i++){
							int b=bucket.pointers[i];
							self_print(b,loc,st);
						}
						next=bucket.next_bucket;
					}
				}else{
					DEBUG_VAR(block);
					self_print(block,loc,st);
				}
				return 1;
			}
		}
		if(ct->op==OP_BIGGER||ct->op==OP_BIGGER_EQUAL){
			if(!ct->right_col){
				int block=bpt.find(ct->right_num,bpt.hdr.root);
				DEBUG_VAR(block);
				// print the initial leaf block 
				leaf_node l_node;
				bpt.get_block(block,(char*)&l_node);
				int i=0;
				for(;i<l_node.size;i++){//find where to start
					int k=l_node.keys[i];
					DEBUG_VAR(k);
					if(ct->op==OP_BIGGER){
						if(k>(ct->right_num))
							break;
					}else{
						if(k>=(ct->right_num))
							break;
					}
				}
				//i could be 4, then the following chunk is skipped
				int ptr=-1;
				int b=-1;
				for(int x=i;x<l_node.size;x++){
					if(l_node.pointers[x]!=ptr)
						ptr=l_node.pointers[x];
					else
						continue;
					if(bpt.hdr.duplicate=='Y'){
						ptr_bucket bucket;
						int nxt=ptr;
						DEBUG_VAR(nxt);
						while(nxt){
							bpt.get_block(nxt,(char*)&bucket);
							for(int l=0;l<bucket.in_use;l++){
								if(bucket.pointers[l]!=b)
									b=bucket.pointers[l];
								else
									continue;
								self_print(b,loc,st);
							}
							nxt=bucket.next_bucket;
						}
					}else{
						DEBUG_VAR(ptr);
						self_print(ptr,loc,st);
					}
				}
				// print the next leaf block and so on
				int next=l_node.next_neighbor;
				while(next){//go to the right to the end
					bpt.get_block(next,(char*)&l_node);
					for(int j=0;j<l_node.size;j++){
						if(l_node.pointers[j]!=ptr)
							ptr=l_node.pointers[j];
						else
							continue;
						if(bpt.hdr.duplicate=='Y'){
							ptr_bucket bucket;
							int next_=ptr;
							DEBUG_VAR(next);
							while(next_){
								bpt.get_block(next_,(char*)&bucket);
								for(int l=0;l<bucket.in_use;l++){
									if(bucket.pointers[l]!=b)
										b=bucket.pointers[l];
									else
										continue;
									self_print(b,loc,st);
								}
								next_=bucket.next_bucket;
							}
						}else{
							DEBUG_VAR(ptr);
							self_print(ptr,loc,st);
						}
					}
					next=l_node.next_neighbor;
				}
				return 1;
			}
		}
		if(ct->op==OP_LESS || ct->op==OP_LESS_EQUAL){
			/**
			 * The smallest value is always in block one
			 * However this may not be true if deletion is added
			**/
			if(!ct->right_col){
				leaf_node l_node;
				int block=bpt.find(ct->right_num,bpt.hdr.root);
				bpt.get_block(block,(char*)&l_node);
				int ptr=-1;
				int b=-1;
				if(block==1){
					DEBUG_VAR(block);
					for(int i=0;i<l_node.size;i++){
						int k=l_node.keys[i];
						if(ct->op==OP_LESS){
							if(k>(ct->right_num)||k==ct->right_num) break;
						}else{
							if(k>(ct->right_num)) break;
						}
						//else print
						if(l_node.pointers[i]!=ptr)
							ptr=l_node.pointers[i];
						else
							continue;
						if(bpt.hdr.duplicate=='Y'){
							ptr_bucket bucket;
							int next=ptr;
							DEBUG_VAR(next);
							while(next){
								bpt.get_block(next,(char*)&bucket);
								for(int j=0;j<bucket.in_use;j++){
									if(bucket.pointers[j]!=b)
										b=bucket.pointers[j];
									else
										continue;
									self_print(b,loc,st);
								}
								next=bucket.next_bucket;
							}
						}else{
							DEBUG_VAR(ptr);
							self_print(ptr,loc,st);
						}
					}
				}else{// read from the beginning to the end
					DEBUG_VAR(block);
					int next=1;
					int done=0;
					while(!done&&next){	
						bpt.get_block(next,(char*)&l_node);
						for(int i=0;i<l_node.size;i++){
							int k=l_node.keys[i];
							if(ct->op==OP_LESS){
								if(k>(ct->right_num)||k==ct->right_num){
									done=1;
									break;
								}
							}else{
								if(k>(ct->right_num)){
									DEBUG_VAR(k);
									done=1;
									break;
								}
							}
							DEBUG_VAR(k);
							//else print
							if(l_node.pointers[i]!=ptr)
								ptr=l_node.pointers[i];
							else
								continue;
							if(bpt.hdr.duplicate=='Y'){
								ptr_bucket bucket;
								int next_=ptr;
								DEBUG_VAR(next);
								while(next_){
									bpt.get_block(next_,(char*)&bucket);
									for(int j=0;j<bucket.in_use;j++){
										if(bucket.pointers[j]!=b)
											b=bucket.pointers[j];
										else
											continue;
										self_print(b,loc,st);
									}
									next_=bucket.next_bucket;
								}
							}else{
								DEBUG_VAR(ptr);
								self_print(ptr,loc,st);
							}
						}
						next=l_node.next_neighbor;
					}
				}
				return 1;
			}
		}
	}
// do linear scan if it gets here
/*
	using namespace std;
	Table &table=cat->tables[st->table];	
	int totl_bl=ceil(table.insertPos/48.0);
	int rowWidth=table.numOfAttrs;
	int recsInBlock=12/rowWidth;
	DEBUG_VAR(totl_bl);
	DEBUG_VAR(rowWidth);
	DEBUG_VAR(recsInBlock);
	int len=length_id_list(st->fields);
	if(loc==NULL){
		for(int i=0;i<rowWidth;i++){	
			cout<<setw(13)<<left<<table.attributes[i];
		}
	}else{
		id_list_t * id_lst = st->fields;	
		while(id_lst){
			cout<<setw(13)<<left<<id_lst->id;
			id_lst = id_lst -> next;
		}
	}
	cout<<"\n";
*/
	// the linear scan
	for(int i=0;i<totl_bl;i++){
		int* dat=(int*)buf_mng->get_buf_block(st->table,i);
		int datChunk[rowWidth];
		int offset=0;
		for(int j=0;j<recsInBlock;j++){
			memcpy(datChunk,dat+offset,rowWidth*sizeof(int));
			if(datChunk[0]==0xC3C3C3C3){
				DEBUG_VAR(datChunk[0]);
				break;
			}
			if(!filter(st,datChunk)){
				offset+=rowWidth;
				MSG("Didn't pass");
				continue;
			}else{
				if(loc==NULL){
					MSG("flag");
					for(int k=0;k<rowWidth;k++)
						cout<<setw(13)<<left<<datChunk[k];
				}else{
					MSG("flag");
					for(int k=0;k<len;k++){
						cout<<setw(13)<<left<<datChunk[loc[k]];
					}	
				}
				cout<<"\n";
			}
			offset+=rowWidth;
		}
	}
	return 1;
}

int self_print(int block, int* loc, select_statement_t* st)
{
	MSG("self_print");
	using namespace std;
	Table &table = cat->tables[st->table];	
	int rowWidth=table.numOfAttrs;
	int len=length_id_list(st->fields);
/*
	if(loc==NULL){
		for(int i=0;i<rowWidth;i++){	
			cout<<setw(13)<<left<<table.attributes[i];
		}
	}else{
		id_list_t* id_lst=st->fields;	
		while(id_lst){
			cout<<setw(13)<<left<<id_lst->id;
			id_lst=id_lst->next;
		}
	}
	cout<<"\n";
*/
	int* dat = (int*)buf_mng->get_buf_block(st->table, block);
	if(dat==NULL)
		return 0;
	int offset=0;
	int datChunk[rowWidth];	
	int numOfRecs=12/rowWidth;	
	for(int i=0;i<numOfRecs;i++){
		memcpy(datChunk,dat+offset,rowWidth*sizeof(int));	
#ifdef DEBUGX
		cout<<"data chunk: ";
		for(int l=0;l<rowWidth;l++)
			cout<<datChunk[l]<<",";
		cout<<"\n";
#endif
		if(datChunk[0]==0xC3C3C3C3)
			break;
		if(!filter(st,datChunk)){
			offset+=rowWidth;
			MSG("Didn't pass");
			continue;
		}
		if(loc==NULL){
			MSG("select *");
			for(int j=0;j<rowWidth;j++)
				cout<<setw(13)<<left<<datChunk[j];
		}else{
			MSG("select ...");
			for(int j=0;j<len;j++)
				cout<<setw(13)<<left<<datChunk[loc[j]];
		}	
		offset+=rowWidth;
		cout<<"\n";
	}	
}

int filter(select_statement_t* st, int* dat)
{
	condition_t* cur=st->conditions;
	int result=1;
	while(cur&&result){
		int left_val, right_val, offset;
		offset=cat->search_attr1(st->table, cur->left_col);
		left_val=dat[offset];
		if(cur->right_col==NULL){
			right_val=cur->right_num;
		}else{
			offset=cat->search_attr1(st->table, cur->right_col);
			right_val=dat[offset];
		}
		DEBUG_VAR(cur->op);
		switch(cur->op){
			case OP_EQUAL:
				result*=(left_val==right_val?1:0);	 
				break;
			case OP_BIGGER:
				result*=(left_val>right_val?1:0);	 
				break;
			case OP_LESS:
				result*=(left_val<right_val?1:0);	 
				break;
			case OP_BIGGER_EQUAL:
				result*=(left_val>=right_val?1:0);	 
				break;
			case OP_LESS_EQUAL:
				result*=(left_val<=right_val?1:0);	 
				break;
			case OP_NOT_EQUAL:
				result*=(left_val!=right_val?1:0);	 
				break;
			default:
				result=0;
				break;
		}
		cur=cur->next;	
	}
	return result;
}

int print_table_fast(char *table_name)
{
	using namespace std;
	if(cat->search_table(table_name)){
		File file(table_name, "r");
		Table &table = cat->tables[table_name];
		int num_attrs = table.numOfAttrs;	
		for(int i=0;i<num_attrs;i++){
			cout<<setw(13)<<left<<table.attributes[i];
		}
		cout<<"\n";
		file.direct_print(num_attrs);
		return 1;
	}
	return 0;
}

//do not flush the table when drop.
int drop_table(char * table)
{
	if(cat->search_table(table)){
		File file;
		Table &tb=cat->tables[table];	
		// delete index file if there is any
		if(tb.numOfIndex>0){
			std::map<std::string,std::string>::iterator it;
			it=tb.index_name.begin();
			for(;it!=tb.index_name.end();++it){
				file.dele_file(table,it->second.c_str());
			}
		}
		// delete table file
		file.dele_file(table);	
		// delete catalog entry
		cat->del_rec(table);
	}else{
		printf("NO such table!\n");	
	}
	return 0;
}

int* produce_dat(num_list_t *values, int count)
{
	int* dat=(int*)calloc(1,count*sizeof(int));
	num_list_t *src_ptr=values;
	int i=0;
	do{
		dat[i]=src_ptr->num;
		src_ptr=src_ptr->next;	
		i++;
	}while(src_ptr);
	return dat;
}

