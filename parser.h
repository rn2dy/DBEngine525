#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C"{
#endif
/**
 * A singly linked-list of identifiers (either table names or column names)
 */
typedef struct id_list_t{
   char* id;
   struct id_list_t* next;
} id_list_t;

/**
 * A singly-linked list of numbers.
 */
typedef struct num_list_t{
   int num;
   struct num_list_t* next;
} num_list_t;

typedef enum operator_t{
   OP_EQUAL,
   OP_BIGGER,	
   OP_LESS,
   OP_BIGGER_EQUAL,
   OP_LESS_EQUAL,	
   OP_NOT_EQUAL	
} operator_t;

/**
 * This is a linked list of conditions in the WHERE clause
 * left_col is the column name that appeared on the left side of the operator
 * op is the operator (< = > != <= >=)
 * right_col is the column name that appeared on the right side of the operator
 * if right_col is null, then the parser found a comparison to a number (e.g. WHERE A=3)
 * next points to the next condition in the list
 */
typedef struct condition_t{
   char* left_col;
   operator_t op;
   char* right_col;
   int right_num;
   struct condition_t* next;
} condition_t;


/**
 * This holds all of the information that was in a SELECT statement.
 * When we introduce joins in assignment 3, I will introduce a new data
 * structure (join_statement_t) to hold the parse tree for those.
 */
typedef struct select_statement_t{
   /// a singly-linked list of the fields listed in the SELECT clause
   /// This will be NULL in SELECT * statements.
   id_list_t* fields;
   /// the name of the table in the FROM clause
   char* table;
   /// a singly-linked list of the consitions in the WHERE clause.
   /// This will be null if there was no WHERE clause.
   condition_t* conditions;
} select_statement_t;


/**
 * This holds all of the information that was in an INSERT statement.
 */
typedef struct insert_statement_t{
   /// the name of the table to insert into
   char* table;
   /// a singly-list of the numbers to insert
   num_list_t* values;
} insert_statement_t;


/**
 * This holds all of the information that was in a CREATE TABLE statement.
 */
typedef struct create_table_statement_t{
   /// the name of the table to create
   char* table;
   /// a singly-list of column names
   id_list_t* columns;
} create_table_statement_t;

/**
 * Create index statement
**/
typedef struct create_index_statement_t{
	char* index;
	char* table;
	char* column;
	int duplicates;
}create_index_statement_t;

typedef struct index_ref_t{
	char* index;
	char* table;
}index_ref_t;
/**
 *  This enum defines a bunch of commands that take no parameters.
 *  If the parser found some command not listed here, then the type
 *  CMD_NONE is used to indicate that none of the parameterless commands should
 *  be run.
 */
typedef enum parameterless_statement_t{
   CMD_NONE=0,
   CMD_PRINT_CATALOG,
   CMD_PRINT_BUFFER,
   CMD_PRINT_HIT_RATE,
   CMD_COMMIT,
   CMD_EXIT
} parameterless_statement_t;

typedef enum variable_t{
   CONFIG_TIMER,
   CONFIG_INDEX_DEBUG
} variable_t;

typedef struct set_statement_t{
   variable_t variable;
   //value is a boolean -- 0 means "off" and nonzero means "on"
   int value;
} set_statement_t;

typedef struct statement_t{
   select_statement_t* select;
   create_table_statement_t* create_table;
   create_index_statement_t* create_index;
   insert_statement_t* insert;
   char* drop_table;   
   index_ref_t* drop_index;
   char* print_table;
   index_ref_t* print_index;
   set_statement_t* set;
   parameterless_statement_t parameterless;
} statement_t;


/**
 * Returns the length of an identifier list
 */
int length_id_list(const id_list_t *);

/**
 * Returns the length of a list of numbers
 */
int length_num_list(const num_list_t*);

/**
 * Frees a parsed SQL statement, and all of the objects created inside it.
 * This includes all strings created in side it. If you want to use them persistently,
 * you call strdup() on them (or copy them to a std::string) and save the duplicate.
 */
void free_statement(statement_t* stmt);

/**
 * This is the main entry point for the parser 
 */
statement_t * parse_statement(char * stmt);

#ifdef SQL_PARSER_INTERNAL
// CS-525 students don't need to ever call these
id_list_t* new_id_list(char* id);
num_list_t* new_num_list(int num);
condition_t* new_condition(char* left_col, operator_t op, char* right_col, int num);
select_statement_t* new_select_statement(id_list_t* fields, char* table, condition_t* conds);
insert_statement_t* new_insert_statement(char* table, num_list_t* values);
create_index_statement_t* new_create_index_statement(char*index,char*table,char*column,int duplicates);
index_ref_t* new_index_ref(char*index,char*table);
create_table_statement_t* new_create_table_statement(char* table, id_list_t* columns);
set_statement_t* new_set_statement(variable_t variable, int value);
statement_t* new_statement(void);

// CS-525 students don't need to ever call these
void free_id_list(id_list_t *);
void free_num_list(num_list_t*);
void free_condition(condition_t *);
void free_select_statement(select_statement_t*);
void free_insert_statement(insert_statement_t*);
void free_create_table_statement(create_table_statement_t*);
void free_set_statement(set_statement_t*);
void free_index_ref(index_ref_t*);
void free_create_index_statement(create_index_statement_t*);

// the statement returned by parsing a single line of SQL
extern statement_t* returned_statement;
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
