#ifndef CATALOG
#define CATALOG

#include <map>
#include <string>
#include <stdlib.h>
#include "table.h"
#include <boost/serialization/map.hpp>

#define TABLE_NAME_LIMIT 3
#define MAX_TABLES 50
#define CATALOG_FILE "catalog.dat"

class Catalog
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & tables;
		ar & numOfTables;
	}
public:
	Catalog();
	~Catalog();
	// updating method
	int add_rec(const char* table, const id_list_t * attrlist);
	int del_rec(char * table);
	int add_rec_count(const char *table);
	int cat_create_index(const char*table,const char*index,const char*column);
	int cat_drop_index(const char*table,const char*index);

	// lookup method
	int* get_index_attr(const char* table, int numOfIndex);
	int cat_check_index(const char*table, const char*index,const char* column);
	int cat_check_index_col(const char*table, const char* col);
	int search_table(const char * table);
	int search_attr1(const char * table, const char * attr_name);
	int search_attr2(const char * table, const id_list_t * alst, int *loc); 
	int get_num_rec(const char * table);
	int get_num_attrs(const char * table);

	// inspection method
	void print_catalog();
	void cat_print_index();

	//class fields
	std::map<std::string, Table> tables;
	unsigned short numOfTables;
};
#endif
	
