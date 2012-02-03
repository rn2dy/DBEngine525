#ifndef TABLE
#define TABLE

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "parser.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#define ATTR_NUM_LIMITS 8

class Table
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & numOfRec;
		ar & numOfAttrs;
		ar & numOfIndex;
		ar & name;
		ar & index_pos;
		ar & index_name;
		ar & attributes;
		ar & insertPos;
	}
public:
	Table();
	Table(const char * table_name, const id_list_t * attrlist);
	~Table();
	
	void print_table();
	int setAttrName(const char * oldstr, const char * newstr);
	int getAttrsIndex(const char *);

	unsigned int numOfRec;
	unsigned short numOfAttrs;
	unsigned short numOfIndex;

	std::string name;
	//std::map<std::string,std::string> index;
	std::map<std::string,int> index_pos;
	std::map<std::string,std::string> index_name;
	std::vector<std::string> attributes;

	int insertPos;
};
#endif
