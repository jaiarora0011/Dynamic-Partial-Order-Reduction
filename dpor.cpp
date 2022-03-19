#include "dpor.h"
#include <assert.h>

using namespace std;

void
concurrent_procs::check_distinct_instruction_labels()
{
  unordered_set<string> instruction_label_set;
    for (auto const& proc : m_procs) {
      for (auto const& ins: proc.second->get_instruction_list()) {
        if (instruction_label_set.count(ins->get_instruction_label())) {
          throw "Instruction Labels for all processes should have unique labels";
        } else {
          instruction_label_set.insert(ins->get_instruction_label());
        }
      }
    }
}

bool
are_instructions_dependant(instruction* i1, instruction* i2)
{
  if (i1->get_instruction_type() != i2->get_instruction_type()) {
    return false;
  }
  if (i1->get_instruction_type() == assignment) {
    auto a1 = dynamic_cast<assignment_instruction*>(i1);
    auto a2 = dynamic_cast<assignment_instruction*>(i2);
    assert(a1);
    assert(a2);
    return a1->are_dependant(a2);
  } else {
    auto a1 = dynamic_cast<mutex_instruction*>(i1);
    auto a2 = dynamic_cast<mutex_instruction*>(i2);
    assert(a1);
    assert(a2);
    return a1->are_dependant(a2);
  }
}

binary_label_relation
compute_dependant_instructions(process* p1, process* p2)
{
  binary_label_relation ret;
  for (auto const& i1: p1->get_instruction_list()) {
    for (auto const& i2: p2->get_instruction_list()) {
      if (are_instructions_dependant(i1, i2)) {
        string l1 = i1->get_instruction_label();
        string l2 = i2->get_instruction_label();
        ret.add_pair(l1, l2);
        ret.add_pair(l2, l1);
      }
    }
  }

  return ret;
}

binary_label_relation
concurrent_procs::compute_dependancy_relation()
{
  if (m_dependancy_relation.size()) {
    return m_dependancy_relation;
  }
  vector<string> process_ids;
  for (auto const& m : m_procs) {
    process_ids.push_back(m.first);
  }

  int p = process_ids.size();
  for (int i = 0; i < p; ++i) {
    string pid_i = process_ids[i];
    for (int j = i + 1; j < p; ++j) {
      string pid_j = process_ids[j];
      auto Dij = compute_dependant_instructions(m_procs[pid_i], m_procs[pid_j]);
      m_dependancy_relation.relation_union(Dij);
    }
  }

  m_dependancy_relation.relation_union(m_program_order);

  return m_dependancy_relation;
}