#ifndef QUERY_H
#define QUERY_H

#include <math.h>
#include "parser.h"
#include "catalog.h"
#include "buffer_mng.h"
#include "file.h"
#include "bplustree.h"
#include "debug.h"

extern Buffer_Manager *buf_mng;
extern Catalog *cat;
/**
 *
**/
void dispatch_query(statement_t * st);

/**
 *
**/
int create_table(create_table_statement_t *);

/**
 * Index create,drop,print statement 
**/
int create_index(create_index_statement_t *);
int drop_index(index_ref_t*);
int drop_index(const char* table, const char*index);
int print_index(index_ref_t*);

/**
 *
**/
int insert(insert_statement_t *);

/**
 *
**/
int select(select_statement_t *);

/**
 *
**/
int print_table1(select_statement_t *st);
int print_table2(select_statement_t *st, int *loc);
int print_table3(select_statement_t *st, int *loc);
int print_table_fast(char *table_name);
int self_print(int block,int* loc,select_statement_t* st);
/**
 *
**/
int drop_table(char *table);
/**
 *
**/
/**
 * Utilities
**/
int check_condition(select_statement_t* st);
int filter(select_statement_t* st, int *dat);
int* produce_dat(num_list_t *values, int count);

#endif
