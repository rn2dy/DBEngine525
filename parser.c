#define SQL_PARSER_INTERNAL
#include "parser.h"
#include <stdlib.h>
#include <ctype.h>
#include "sql.tab.h"
#include "sql.lex.h"

int yyparse(void);

statement_t* returned_statement;

//user-visible functions
int length_id_list(const id_list_t *it){
   int i=0;
   while(it){
      ++i;
      it=it->next;
   }
   return i;
}
int length_num_list(const num_list_t *it){
   int i=0;
   while(it){
      ++i;
      it=it->next;
   }
   return i;
}
void free_statement(statement_t* i){
   if(!i) return;
   free_select_statement(i->select);
   free_create_table_statement(i->create_table);
   free_create_index_statement(i->create_index);
   free_insert_statement(i->insert);
   free_index_ref(i->drop_index);
   free_index_ref(i->print_index);
   free(i->drop_table);
   free(i->print_table);
   free_set_statement(i->set);
   free(i);
}
statement_t * parse_statement(char * input){
   char* t;
   for (t=input;*t;++t) *t=tolower(*t);
   yy_scan_string(input);
   if (yyparse()==0)
      return returned_statement;
   else
      return NULL;
}


//internal parser functions
id_list_t* new_id_list(char* id){
   id_list_t* i=malloc(sizeof(id_list_t));
   i->id=id;
   i->next=NULL;
   return i;
}
num_list_t* new_num_list(int num){
   num_list_t* i=malloc(sizeof(num_list_t));
   i->num=num;
   i->next=NULL;
   return i;
}
condition_t* new_condition(char* left_col, operator_t op, char* right_col, int right_num){
   condition_t* i=malloc(sizeof(condition_t));
   i->left_col=left_col;
   i->op=op;
   i->right_col=right_col;
   i->right_num=right_num;
   i->next=NULL;
   return i;
}
select_statement_t* new_select_statement(id_list_t* fields, char* table, condition_t* conds){
   select_statement_t* i=malloc(sizeof(select_statement_t));
   i->fields=fields;
   i->table=table;
   i->conditions=conds;
   return i;
}
insert_statement_t* new_insert_statement(char* table, num_list_t* values){
   insert_statement_t* i=malloc(sizeof(insert_statement_t));
   i->table=table;
   i->values=values;
   return i;
}
create_table_statement_t* new_create_table_statement(char* table, id_list_t* columns){
   create_table_statement_t* i=malloc(sizeof(create_table_statement_t));
   i->table=table;
   i->columns=columns;
   return i;
}
create_index_statement_t* new_create_index_statement(char* index, char* table, char* column, int duplicates){
   create_index_statement_t* i=malloc(sizeof(create_index_statement_t));
   i->index=index;
   i->table=table;
   i->column=column;
   i->duplicates=duplicates;
   return i;
}
index_ref_t* new_index_ref(char* index, char* table){
   index_ref_t* i=malloc(sizeof(index_ref_t));
   i->index=index;
   i->table=table;
   return i;
}
set_statement_t* new_set_statement(variable_t variable, int value){
   set_statement_t* i=malloc(sizeof(set_statement_t));
   i->variable=variable;
   i->value=value;
   return i;
}
statement_t* new_statement(void){
   statement_t* i=malloc(sizeof(statement_t));
   i->select=NULL;
   i->create_table=NULL;
   i->create_index=NULL;
   i->insert=NULL;
   i->drop_table=NULL;   
   i->drop_index=NULL;
   i->print_table=NULL;
   i->print_index=NULL;
   i->set=NULL;
   i->parameterless=CMD_NONE;
   return i;
}

void free_id_list(id_list_t *i){
   id_list_t* temp;
   while(i){
      temp=i->next;
      free(i->id);
      free(i);
      i=temp;
   }
}
void free_num_list(num_list_t* i){
   num_list_t* temp;
   while(i){
      temp=i->next;
      free(i);
      i=temp;
   }
}
void free_condition(condition_t * i){
   condition_t* temp;
   while(i){
      temp=i->next;
      free(i->left_col);
      free(i->right_col);
      free(i);
      i=temp;
   }
}
void free_select_statement(select_statement_t* i){
   if(!i) return;
   free_id_list(i->fields);
   free(i->table);
   free_condition(i->conditions);
   free(i);
}
void free_insert_statement(insert_statement_t* i){
   if(!i) return;
   free(i->table);
   free_num_list(i->values);
   free(i);
}
void free_create_table_statement(create_table_statement_t* i){
   if(!i) return;
   free(i->table);
   free_id_list(i->columns);
   free(i);
}
void free_create_index_statement(create_index_statement_t* i){
   if(!i) return;
   free(i->index);
   free(i->table);
   free(i->column);
   free(i);
}
void free_index_ref(index_ref_t* i){
   if(!i) return;
   free(i->index);
   free(i->table);
   free(i);
}
void free_set_statement(set_statement_t* i){
   if(!i) return;
   free(i);
}

