%{
#include <cstdio>
#include <iostream>
#include <vector>
#include <unordered_set>
#include "program.hpp"
using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
concurrent_procs* parsed = NULL;
 
void yyerror(const char *s);
%}


%union{
	char* stringVal;
	int intVal;
  instruction* ins;
  assignment_instruction* assignIns;
  mutex_instruction* mutexIns;
  process* proc;
  concurrent_procs* concProcs;
  binary_label_relation* po;
}

%token <intVal>	I_CONSTANT
%token <stringVal>	IDENTIFIER
%token	ASSIGN PO RELEASE ACQUIRE

%nterm <concProcs> start_sym program_list
%nterm <proc> program instruction_list
%nterm <ins> instruction ins_
%nterm <po> program_order_relation pair_list s_pair

%start start_sym
%%

start_sym
  : program_list program_order_relation { $1->set_program_order(*$2); $$ = $1; parsed = $$; }
  ;

program_list
  : program { $$ = new concurrent_procs(); $$->add_program($1); }
  | program_list program  { $1->add_program($2); $$ = $1; }
  ;

program
  : IDENTIFIER '{' instruction_list '}' { $3->set_process_label($1); $$ = $3; }
  ;

instruction_list
  : instruction { $$ = new process({$1}); }
  | instruction_list instruction  {$1->add_instruction($2); $$ = $1; }
  ;

instruction
  : IDENTIFIER ':' ins_  { $3->set_label($1); $$ = $3; }
  ;

ins_
  : IDENTIFIER ASSIGN I_CONSTANT  { $$ = new assignment_instruction($1, $3); }
  | IDENTIFIER ASSIGN IDENTIFIER  { $$ = new assignment_instruction($1, $3); }
  | RELEASE '(' IDENTIFIER ')'  { $$ = new mutex_instruction($3, false); }
  | ACQUIRE '(' IDENTIFIER ')'  { $$ = new mutex_instruction($3, true); }
  ;

program_order_relation
  : PO ':' '{' pair_list '}'  { $$ = $4; }
  ;

pair_list
  : s_pair  { $$ = $1; }
  | pair_list ',' s_pair  { $1->relation_union(*$3); $$ = $1; }
  |                       { $$ = new binary_label_relation(); }
  ;

s_pair
  : '(' IDENTIFIER ',' IDENTIFIER ')' {$$ = new binary_label_relation(); $$->add_pair($2, $4); }
  ;

%%
#include <stdio.h>

void yyerror(const char *s)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}
