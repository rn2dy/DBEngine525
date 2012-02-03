#include "catalog.h"

Catalog::Catalog()
:numOfTables(0)
{}

Catalog::~Catalog()
{}

/***************************INDEX****************************/

int Catalog::cat_check_index(const char* table,const char* index,const char*column)
{
	Table &tb=tables[table];
	if(tb.numOfIndex==0)
		return 0;
	if(tb.index_name.find(index)!=tb.index_name.end()){
		return -1;
	}
	std::map<std::string,std::string>::iterator it=tb.index_name.begin();
	for(;it!=tb.index_name.end();++it){
		if(column==NULL){
			return -3;
		}
		if(it->second==column){
			return -2;
		}
	}
}

int Catalog::cat_check_index_col(const char*table, const char*col)
{
	Table &tb=tables[table];
	std::map<std::string,std::string>::iterator it;
	for(it=tb.index_name.begin();it!=tb.index_name.end();it++){
		if(it->second==col)
			return 1;
	}
	return 0;
}

int Catalog::cat_create_index(const char* table,const char* index, const char* column)
{
	Table &tb=tables[table];
	int num=tb.numOfIndex;
	if(num==2)
		return 0;
	if(num<2){
		tb.index_pos[index]=tb.getAttrsIndex(column);
		tb.index_name[index]=column;
		tb.numOfIndex++;
		return 1;
	}
	//else	
	return 0;
}

int* Catalog::get_index_attr(const char* table, int numOfIndex)
{
	Table &tb=tables[table];
	int*dat=new int[numOfIndex];
	std::map<std::string,int>::iterator it;
	it=tb.index_pos.begin();
	int i=0;
	for(;it!=tb.index_pos.end();it++){
		dat[i]=it->second;
		i++;
	}
	return dat;
}

int Catalog::cat_drop_index(const char* table, const char* index)
{
	Table &tb=tables[table];
	int num=tb.numOfIndex;
	if(num==0)
		return 0;
	if(num>0){
		tb.index_name.erase(index);
		tb.index_pos.erase(index);
		tb.numOfIndex--;
		return 1;	
	}
}
void Catalog::cat_print_index()
{
	std::map<std::string, Table>::iterator it;
	for(it=tables.begin();it!=tables.end();it++){
		Table &table = it->second;
		if(table.numOfIndex==0)
			std::cout<<"table "<<table.name<<" has no index"<<std::endl;
		//else
		std::map<std::string,std::string>::iterator itt;
		for(itt=table.index_name.begin();itt!=table.index_name.end();itt++)
			std::cout<<"table "<<table.name<<", index "<<itt->first<<" on "<<itt->second<<std::endl;
	}
}
/********************* Index function end ************************/

int Catalog::add_rec(const char* table_name, const id_list_t * attrlist)
{
	int count=0;
	const char * ptr=table_name;
	while(*ptr){
		ptr++;
		count++;
	}
	if(count>3){
		std::cout <<"Table name longer than 3!\n";
		return 0;
	}
	if(tables.find(table_name) != tables.end())
	{
		std::cout << "Table with the same name already existed!\n";
		return 0;
	}
	//else
	if(numOfTables < MAX_TABLES)
	{
		
		tables[table_name] = Table(table_name, attrlist);
		numOfTables++;
		return 1;
	}
	return 0;
}
// return the insertion position of the new record
int Catalog::add_rec_count(const char* table_name)
{
	Table &tb=tables[table_name];
	int recSize=tb.numOfAttrs*sizeof(int);
	int curBlocks=ceil(tb.insertPos/48.0);
	int inBlockSize=tb.insertPos%48+recSize;	
	int newInsertPos=0;
	if(inBlockSize>48){
		newInsertPos=curBlocks*48;
	}else{
		newInsertPos=tb.insertPos;
	}
	tb.insertPos=newInsertPos+recSize;
	tb.numOfRec++;
	return newInsertPos;
}

/**
 * Return an 1 if the table exist, 0 if not 
**/
int Catalog::search_table(const char * table_name)
{
	if(tables.find(table_name)==tables.end())
		return 0;
	else
		return 1;
}

/**
 * Return an index that indicates the position of the attribute, or -1 
 * the attributes does not exit 
**/
int Catalog::search_attr1(const char *table_name, const char * attr_name)
{
	return tables[table_name].getAttrsIndex(attr_name);	
}

/**
 * Return a list of indices of the positions of the attribute,
 * or NULL if there is one or more non-exist attributes 
**/
int Catalog::search_attr2(const char* table_name, const id_list_t * attrlist, int * loc)
{
	Table &table = tables[table_name];
	const id_list_t * tmp_lst = attrlist; 	
	int i = 0;
	do{
		int p = table.getAttrsIndex(tmp_lst->id);	
		if(p == -1){
			return 0;
		}
		*loc=p;
		loc++;
		tmp_lst = tmp_lst -> next;
	}while(tmp_lst!=NULL);
	return 1;
}

int Catalog::get_num_rec(const char * table)
{
	return tables[table].numOfRec;
}

int Catalog::get_num_attrs(const char * table)
{
	return tables[table].numOfAttrs;
}

/**
 * Return 1 if successfully delete, 0 otherwise
**/
int Catalog::del_rec(char *table_name)
{
	if(search_table(table_name))
	{
		tables.erase(tables.find(table_name));		
		numOfTables--;
		return 1;
	}
	else
		return 0;
}

void Catalog::print_catalog()
{
	std::cout << "There are " << numOfTables << " table(s) in the DB\n";
	std::map<std::string, Table>::iterator it;
	for(it=tables.begin(); it!=tables.end();it++){
		((*it).second).print_table();	
		std::cout<<"\n";
	}
}
