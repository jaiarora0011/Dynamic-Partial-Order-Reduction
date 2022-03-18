#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <assert.h>
#include "program.h"
#include "parse.tab.hpp"

extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
extern "C" concurrent_procs* parsed;

using namespace std;

int
main(int argc, char **argv)
{ 
  if (argc == 1) {
    cout << "Insufficient number of Input Parameters. Expected = 2. Found = " << argc << endl;
    return 1; 
  }
  
  char const *filename = argv[1];
  yyin = fopen(filename, "r");
  assert(yyin);
  int ret = yyparse();
  printf("retv = %d\n", ret);
  assert(parsed);
  cout << parsed->dump_string() << endl;

  return 0;
}