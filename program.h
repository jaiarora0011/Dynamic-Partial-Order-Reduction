#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
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

  void add_instruction(instruction* const& other)
  {
    m_list.push_back(other);
  }

  void set_process_label(string label)
  {
    m_process_label = label;
  }

  string get_process_label()
  {
    return m_process_label;
  }

  vector<instruction*> get_instruction_list()
  {
    return m_list;
  }

  void sync_process_label_across_instructions()
  {
    for (auto ins : m_list) {
      ins->set_process_id(m_process_label);
    }
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
  unordered_map<string, process*> m_procs;
  binary_label_relation m_program_order;
  binary_label_relation m_dependancy_relation;
  unordered_map<string, string> m_instruction_to_process_map;

public:
  concurrent_procs()
    : m_procs(), m_program_order()
  { }

  void set_program_order(binary_label_relation const& p)
  {
    m_program_order = p;
  }

  void add_program(process* const& other)
  {
    if (m_procs.count(other->get_process_label())) {
      throw "All processes should have unique labels";
    }
    m_procs.insert(make_pair(other->get_process_label(), other));
    other->sync_process_label_across_instructions();
  }

  string dump_string()
  {
    stringstream ss;
    for (auto const& proc : m_procs) {
      ss << proc.second->dump_string() << "\n\n";
    }
    ss << "PROGRAM_ORDER: \n";
    ss << m_program_order.dump_string() << "\n";

    ss << "DEPENDANCY_RELATION: \n";
    ss << m_dependancy_relation.dump_string();

    return ss.str();
  }

  void check_distinct_instruction_labels();
  binary_label_relation compute_dependancy_relation();
};

#endif