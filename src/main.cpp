#include <iostream>
#include <stdio.h>
#include <string>
#include <chrono>
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
  chrono::steady_clock::time_point begin = chrono::steady_clock::now();
  char const *filename = argv[1];
  yyin = fopen(filename, "r");
  assert(yyin);
  int ret = yyparse();
  if (ret) {
    cout << "Error in parsing input" << endl;
    return 1;
  }
  assert(parsed);
  parsed->check_distinct_instruction_labels();
  parsed->compute_dependancy_relation();
  cout << parsed->dump_string() << endl;
  dpor algo(parsed, filename, argv[2]);
  algo.dynamic_por();
  chrono::steady_clock::time_point end = chrono::steady_clock::now();
  cout << "Time difference = " << chrono::duration_cast<chrono::microseconds> (end - begin).count() << "[µs]" << std::endl;
  cout << algo.get_stats() << endl;
  algo.print_to_dot_format();

  return 0;
}
