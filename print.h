#ifndef PRINT_H
#define PRINT_H

#include "parser.h"

#ifdef __cplusplus
extern "C"{
#endif

void print_id_list(id_list_t* i);
void print_num_list(num_list_t* i);
void print_condition(condition_t* i);
void print_select_statement(select_statement_t* i);
void print_insert_statement(insert_statement_t* i);
void print_create_table_statement(create_table_statement_t* i);
void print_set_statement(set_statement_t* i);
void dispatch_print(statement_t* parsed);

#ifdef __cplusplus
}
#endif
#endif
