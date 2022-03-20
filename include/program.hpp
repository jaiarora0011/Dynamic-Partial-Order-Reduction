#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <unordered_map>
#include "instruction.hpp"

using namespace std;

class process
{
private:
  label m_process_label;
  vector<instruction*> m_list;
public:
  process(label label) : m_process_label(label)
  { }

  process(label label, vector<instruction*> const& ins_list)
    : m_process_label(label), m_list(ins_list)
  { }

  process(vector<instruction*> const& ins_list)
    : m_list(ins_list)
  { }

  void add_instruction(instruction* const& other) { m_list.push_back(other); }

  void set_process_label(label label) { m_process_label = label; }

  label get_process_label() { return m_process_label; }

  vector<instruction*> get_instruction_list() { return m_list; }

  void sync_process_label_across_instructions()
  {
    for (auto ins : m_list) {
      ins->set_process_id(m_process_label);
    }
  }

  unordered_set<variable> get_shared_vars()
  {
    unordered_set<variable> ret;
    for (auto const& ins : m_list) {
      if (ins->get_instruction_type() == assignment) {
        auto assign = dynamic_cast<assignment_instruction*>(ins);
        ret.insert(assign->get_lhs());
        if (!assign->is_constant_assignment()) {
          ret.insert(assign->get_rhs_var());
        }
      }
    }
    return ret;
  }

  unordered_set<variable> get_mutex_vars()
  {
    unordered_set<variable> ret;
    for (auto const& ins : m_list) {
      if (ins->get_instruction_type() == mutex) {
        auto mut = dynamic_cast<mutex_instruction*>(ins);
        ret.insert(mut->get_mutex_var());
      }
    }
    return ret;
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
  unordered_map<label, process*> m_procs;
  binary_label_relation m_program_order;
  binary_label_relation m_dependancy_relation;

public:
  concurrent_procs()
    : m_procs(), m_program_order()
  { }

  unordered_map<label, process*> get_processes() { return m_procs; }
  void set_program_order(binary_label_relation const& p) { m_program_order = p; }
  binary_label_relation get_dependant_set() {return m_dependancy_relation; }

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
