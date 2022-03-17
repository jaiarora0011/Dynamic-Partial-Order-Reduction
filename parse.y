%{
#include <cstdio>
#include <iostream>
using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
 
void yyerror(const char *s);
%}


%union{
	char* stringVal;
	int intVal;
}

%token <intVal>	I_CONSTANT
%token <stringVal>	IDENTIFIER
%token	ASSIGN PO RELEASE ACQUIRE

// %nterm <node> primary_expression constant string expression postfix_expression unary_expression
// %nterm <node> cast_expression multiplicative_expression additive_expression shift_expression
// %nterm <node> relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression
// %nterm <node> logical_and_expression logical_or_expression conditional_expression assignment_expression
// %nterm <node> type_specifier declaration_specifiers declarator parameter_type_list parameter_list direct_declarator
// %nterm <node> parameter_declaration expression_statement compound_statement block_item_list block_item
// %nterm <node> declaration statement translation_unit external_declaration function_definition
// %nterm <node> selection_statement iteration_statement jump_statement assignment_operator
// %nterm <node> init_declarator_list init_declarator initializer argument_expression_list type_qualifier
// %nterm <node> pointer

%start start_sym
%%

start_sym
  : program_list program_order_relation
  ;

program_list
  : program
  | program_list program
  ;

program
  : IDENTIFIER '{' instruction_list '}'
  ;

instruction_list
  : instruction
  | instruction_list instruction
  ;

instruction
  : IDENTIFIER ':' ins
  ;

ins
  : IDENTIFIER ASSIGN expression
  | RELEASE '(' IDENTIFIER ')'
  | ACQUIRE '(' IDENTIFIER ')'
  ;

expression
  : I_CONSTANT
  | IDENTIFIER
  ;

program_order_relation
  : PO ':' '{' pair_list '}'
  ;

pair_list
  : pair
  | pair_list ',' pair
  ;

pair
  : '(' IDENTIFIER ',' IDENTIFIER ')'
  ;

%%
#include <stdio.h>

void yyerror(const char *s)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}
