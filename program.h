#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include <vector>
#include <unordered_set>
#include "util.h"
#include "instruction.h"

using namespace std;

class process
{
private:
  string m_process_label;
  vector<instruction> m_list;
public:
  process(string label) : m_process_label(label)
  { }

  process(string label, vector<instruction> ins_list)
    : m_process_label(label), m_list(ins_list)
  { }
};

class concurrent_procs
{
private:
  vector<process> m_procs;
  unordered_set<pair<string, string>, hash<pair<string, string>>> m_program_order;

public:
  concurrent_procs(vector<process> processes)
    : m_procs(processes), m_program_order()
  { }

  concurrent_procs(vector<process> processes, unordered_set<pair<string, string>> po)
    : m_procs(processes), m_program_order(po)
  { }
};

#endif