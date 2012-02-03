%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SQL_PARSER_INTERNAL
#include "parser.h"
void yyerror(const char *);
int yylex(void);
%}

%debug

%token SELECT
%token FROM
%token WHERE
%token AND
%token INSERT
%token INTO
%token VALUES
%token CREATE
%token TABLE
%token PRINT
%token CATALOG
%token BUFFER
%token HIT
%token RATE
%token SET
%token DROP
%token TIMER
%token INDEX
%token NO
%token DUPLICATES
%token OF
%token ON
%token OFF
%token DEBUG
%token COMMIT
%token EXIT
%token <operator_v> OPERATOR
%token <string_v> IDENTIFIER
%token <int_v> NUMBER
%token UNKNOWN_TOKEN

%union{
   char* string_v;
   int int_v;
   struct id_list_t* id_list;
   struct num_list_t* num_list;
   enum operator_t operator_v;
   struct condition_t* condition;
   struct select_statement_t* select_statement;
   struct create_table_statement_t* create_table_statement;
   struct create_index_statement_t* create_index_statement;
   struct index_ref_t* index_ref;
   struct insert_statement_t* insert_statement;
   enum variable_t variable;
   struct set_statement_t* set_statement;
   enum parameterless_statement_t parameterless_statement;
   struct statement_t* statement;
}


%type <int_v> BOOLEAN
%type <id_list> ID_LIST
%type <num_list> NUM_LIST
%type <condition> CONDITION
%type <condition> COND_LIST
%type <id_list> S_FIELDS
%type <select_statement> SELECT_S
%type <create_table_statement> CREATE_T_S
%type <create_index_statement> CREATE_I_S
%type <insert_statement> INSERT_S
%type <variable> VARIABLE
%type <set_statement> SET_S
%type <index_ref> DROP_I_S
%type <index_ref> PRINT_I_S
%type <string_v> DROP_T_S
%type <string_v> PRINT_T_S
%type <parameterless_statement> NOPARAM_S
%type <statement> STATEMENT

%destructor {free($$);} <string_v>
%destructor {free_id_list($$);} <id_list>
%destructor {free_num_list($$);} <num_list>
%destructor {free_condition($$);} <condition>
%destructor {free_select_statement($$);} <select_statement>
%destructor {free_create_table_statement($$);} <create_table_statement>
%destructor {free_create_index_statement($$);} <create_index_statement>
%destructor {free_insert_statement($$);} <insert_statement>
%destructor {free_set_statement($$);} <set_statement>
%destructor {free_index_ref($$);} <index_ref>

%start STATEMENT


%%

ID_LIST	    : IDENTIFIER				        { $$ = new_id_list($1)}
            | IDENTIFIER ',' ID_LIST                            { $$ = new_id_list($1); $$->next = $3}
            ;
NUM_LIST    : NUMBER					        { $$ = new_num_list($1)}
	    | NUMBER ',' NUM_LIST                               { $$ = new_num_list($1); $$->next = $3}
            ;
CONDITION   : IDENTIFIER OPERATOR IDENTIFIER              	{ $$ = new_condition($1,$2,$3,195195195) }
	    | IDENTIFIER OPERATOR NUMBER                        { $$ = new_condition($1,$2,NULL,$3) }
            ;
COND_LIST   : CONDITION                                         { $$ = $1 }
	    | CONDITION AND COND_LIST                           { $$ = $1; $$->next = $3; }
	    ;
S_FIELDS    : ID_LIST					        { $$ = $1 }
	    | '*'					        { $$ = NULL }
            ;
SELECT_S    : SELECT S_FIELDS FROM IDENTIFIER                   { $$ = new_select_statement($2,$4,NULL) }
	    | SELECT S_FIELDS FROM IDENTIFIER WHERE COND_LIST   { $$ = new_select_statement($2,$4,$6)   }
	    ;
INSERT_S    : INSERT INTO IDENTIFIER VALUES '(' NUM_LIST ')'    { $$ = new_insert_statement($3,$6) }
	    ;
CREATE_T_S  : CREATE TABLE IDENTIFIER '(' ID_LIST ')'           { $$ = new_create_table_statement($3,$5) }
	    ;
CREATE_I_S  : CREATE INDEX IDENTIFIER ON IDENTIFIER '(' IDENTIFIER ')'
	                                                        { $$ = new_create_index_statement($3,$5,$7,1) }
            | CREATE INDEX IDENTIFIER ON IDENTIFIER '(' IDENTIFIER ')' NO DUPLICATES
	                                                        { $$ = new_create_index_statement($3,$5,$7,0 )}
	    ;
DROP_T_S    : DROP TABLE IDENTIFIER                             { $$ = $3 }
	    ;
DROP_I_S    : DROP INDEX IDENTIFIER OF TABLE IDENTIFIER         { $$ = new_index_ref($3,$6) }
	    ;
PRINT_T_S   : PRINT TABLE IDENTIFIER			        { $$ = $3 }
	    ;
PRINT_I_S   : PRINT INDEX IDENTIFIER OF TABLE IDENTIFIER        { $$ = new_index_ref($3,$6) }
	    ;
VARIABLE    : TIMER					        { $$ = CONFIG_TIMER }
	    | INDEX DEBUG                                       { $$ = CONFIG_INDEX_DEBUG}
	    ;
BOOLEAN     : ON					        { $$ = 1 }
	    | OFF                                               { $$ = 0 }
	    ;
SET_S       : SET VARIABLE BOOLEAN			        { $$ = new_set_statement($2,$3) }
	    ;
NOPARAM_S   : PRINT CATALOG				        { $$ = CMD_PRINT_CATALOG }
	    | PRINT BUFFER				        { $$ = CMD_PRINT_BUFFER }
            | PRINT HIT RATE                                    { $$ = CMD_PRINT_HIT_RATE }
	    | COMMIT					        { $$ = CMD_COMMIT }
	    | EXIT                                              { $$ = CMD_EXIT }
            ;
STATEMENT   : SELECT_S ';'				        { returned_statement = new_statement(); $$=returned_statement; returned_statement->select=$1;}
	    | INSERT_S ';'                                      { returned_statement = new_statement(); $$=returned_statement; returned_statement->insert=$1;}
	    | CREATE_T_S ';'                                    { returned_statement = new_statement(); $$=returned_statement; returned_statement->create_table=$1;}
	    | CREATE_I_S ';'                                    { returned_statement = new_statement(); $$=returned_statement; returned_statement->create_index=$1;}
	    | DROP_I_S ';'                                      { returned_statement = new_statement(); $$=returned_statement; returned_statement->drop_index=$1;}
	    | PRINT_I_S ';'				        { returned_statement = new_statement(); $$=returned_statement; returned_statement->print_index=$1;}
	    | DROP_T_S ';'                                      { returned_statement = new_statement(); $$=returned_statement; returned_statement->drop_table=$1;}
	    | PRINT_T_S ';'				        { returned_statement = new_statement(); $$=returned_statement; returned_statement->print_table=$1;}
            | SET_S ';'                                         { returned_statement = new_statement(); $$=returned_statement; returned_statement->set=$1;}
            | NOPARAM_S ';'                                     { returned_statement = new_statement(); $$=returned_statement; returned_statement->parameterless=$1;}
	    ;
%%

void yyerror (const char *s){
   fprintf(stderr, "%s\n", s);
}
