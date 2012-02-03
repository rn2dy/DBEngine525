#include "table.h"

Table::Table(const char * table_name, const id_list_t * attrlist)
:name(table_name), numOfAttrs(0),numOfIndex(0),attributes(ATTR_NUM_LIMITS, ""),numOfRec(0),insertPos(0)
{
	int i = 0;
	const id_list_t *ptr = attrlist;
	do{
		attributes[i] = ptr -> id;
		//std::cout << attributes[i] << std::endl;
		i++;
		ptr = ptr -> next;
	}while(ptr != NULL);
	numOfAttrs = i;
}

Table::Table()
{}

Table::~Table()
{}

// depracted function useless....
void Table::print_table()
{
	
	using namespace std;
	cout << "Table: " << name << "  Attributes: ";
	int i = 0; 
	while(attributes[i] != "" && i < ATTR_NUM_LIMITS)
	{
		cout << attributes[i] << " ";
		i++;
	}
	cout <<" Records: "<< numOfRec <<"\n";
	map<string,string>::iterator it=index_name.begin();		
	for(;it!=index_name.end();++it){
		cout<<"Has index: "<<it->first;
		cout<<" on attribute: " << it->second <<endl;
	}
	cout<<"Last position in file: "<<insertPos<<"(Bytes)"<<endl;
}

/**
 * Return 1 means succesfully find and replace the attribute name, 
 * 0 otherwise
**/ 
int Table::setAttrName(const char * oldstr, const char * newstr)
{
	int i=0; 
	for(;i < ATTR_NUM_LIMITS; i++)
	{
		if(attributes[i] == oldstr)
		{
			attributes[i] = newstr;
			return 1;
		}
	}
	return 0;
}

/**
 * Return the index of the attribute within the vector
**/
int Table::getAttrsIndex(const char * attr_name)
{
	int i = 0;	
	for(;i<ATTR_NUM_LIMITS;i++)
	{ 
		if(attributes[i]==attr_name)
			return i;
	}
	return -1;
}

