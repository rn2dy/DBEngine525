%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "sql.tab.h"
void yyerror(const char *);
char *strdup(const char *s);
%}

%option ansi-prototypes noyywrap yy_scan_string

%%

select               return SELECT;
from	             return FROM;
where                return WHERE;
and                  return AND;
insert	             return INSERT;
into                 return INTO;
values               return VALUES;
create               return CREATE;
table                return TABLE;
print                return PRINT;
catalog              return CATALOG;
buffer               return BUFFER;
hit                  return HIT;
rate                 return RATE;
drop		     return DROP;
set                  return SET;
timer                return TIMER;
index		     return INDEX;
no                   return NO;
duplicates           return DUPLICATES;
of		     return OF;
on                   return ON;
off                  return OFF;
debug		     return DEBUG;
commit               return COMMIT;
exit                 return EXIT;
quit                 return EXIT;
"="                  {yylval.operator_v = OP_EQUAL; return OPERATOR;}
"<"                  {yylval.operator_v = OP_LESS; return OPERATOR;}
"<="                 {yylval.operator_v = OP_LESS_EQUAL; return OPERATOR;}
">"                  {yylval.operator_v = OP_BIGGER; return OPERATOR;}
">="                 {yylval.operator_v = OP_BIGGER_EQUAL; return OPERATOR;}
"!="                 {yylval.operator_v = OP_NOT_EQUAL; return OPERATOR;}
"("                  return *yytext;
")"                  return *yytext;
";"                  return *yytext;
","                  return *yytext;
"*"		     return *yytext;
[a-z][a-z0-9]?[a-z0-9]?	     {yylval.string_v = strdup(yytext); return IDENTIFIER;}
[0-9]+               {yylval.int_v = atoi(yytext); return NUMBER;}
[ \t]                {}
\r                   {}
.                    {yyerror("Unknown Character"); return UNKNOWN_TOKEN;}

%%
