#include <iostream>
#include <stdio.h>
#include <string>
#include "dpor.hpp"
#include "parse.tab.hpp"

extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
extern "C" concurrent_procs* parsed;

using namespace std;

int
main(int argc, char **argv)
{ 
  if (argc <= 2) {
    cout << "Insufficient number of Input Parameters. Expected = 3. Found = " << argc << endl;
    return 1; 
  }
  
  char const *filename = argv[1];
  yyin = fopen(filename, "r");
  assert(yyin);
  int ret = yyparse();
  printf("retv = %d\n", ret);
  assert(parsed);
  parsed->check_distinct_instruction_labels();
  parsed->compute_dependancy_relation();
  cout << parsed->dump_string() << endl;
  dpor algo(parsed, filename, argv[2]);
  algo.dynamic_por();
  cout << algo.get_stats() << endl;
  algo.print_to_dot_format();

  return 0;
}