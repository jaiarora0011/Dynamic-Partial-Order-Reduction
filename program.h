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
  vector<instruction*> m_list;
public:
  process(string label) : m_process_label(label)
  { }

  process(string label, vector<instruction*> const& ins_list)
    : m_process_label(label), m_list(ins_list)
  { }

  process(vector<instruction*> const& ins_list)
    : m_list(ins_list)
  { }

  void addInstruction(instruction* const& other)
  {
    m_list.push_back(other);
  }

  void setProcessLabel(string label)
  {
    m_process_label = label;
  }

  string dump_string() const
  {
    stringstream ss;
    ss << m_process_label << ":\n";
    for (auto const& ins : m_list) {
      ss << "\t" << ins->dump_string() << "\n";
    }
    return ss.str();
  }
};

class concurrent_procs
{
private:
  vector<process> m_procs;
  po_rel m_program_order;

public:
  concurrent_procs(vector<process> processes)
    : m_procs(processes), m_program_order()
  { }

  concurrent_procs(vector<process> processes, unordered_set<pair<string, string>> po)
    : m_procs(processes), m_program_order(po)
  { }

  concurrent_procs(vector<process> processes, vector<pair<string, string>> po)
    : m_procs(processes), m_program_order(po)
  { }

  void set_program_order(po_rel p)
  {
    m_program_order = p;
  }

  void addProgram(process other)
  {
    m_procs.push_back(other);
  }

  string dump_string()
  {
    stringstream ss;
    for (auto const& proc : m_procs) {
      ss << proc.dump_string() << "\n\n";
    }
    ss << "PROGRAM_ORDER: \n";
    ss << m_program_order.dump_string(); 

    return ss.str();
  }
};

#endif