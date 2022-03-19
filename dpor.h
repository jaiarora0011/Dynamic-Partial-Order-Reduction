#ifndef DPOR_H
#define DPOR_H

#include "program.h"
#include <assert.h>

using namespace std;

enum mutex_status {
  unlocked,
  locked
};

// string mutex_status_to_string(mutex_status m)
// {
//   if (m == unlocked) {
//     return "unlocked";
//   } else {
//     return "locked";
//   }
// }

class state
{
private:
  label m_label;
  // var --> value mapping
  unordered_map<variable, int> m_shared_state;
  // lock --> (status, owner) mapping
  unordered_map<variable, pair<mutex_status, label>> m_mutex_state;
  // process_id --> pc (to be executed) mapping
  unordered_map<label, int> m_loc_state;
public:
  state()
  { }

  state(string label) : m_label(label)
  { }

  state(state const& other)
    : m_shared_state(other.m_shared_state),
      m_mutex_state(other.m_mutex_state),
      m_loc_state(other.m_loc_state)
  { }

  // Returns the start state with all shared variables
  // initialized to zero, all mutex variables unlocked,
  // and pc's for all processes to zero
  static state get_start_state(
    unordered_set<variable> const& shared_vars,
    unordered_set<variable> const& mutex_vars,
    unordered_map<label, process*> const& processes
  )
  {
    state ret("s0");
    for (auto const& var : shared_vars) {
      ret.m_shared_state.insert(make_pair(var, 0));
    }
    for (auto const& mutex : mutex_vars) {
      ret.m_mutex_state.insert(make_pair(mutex, make_pair(unlocked, "")));
    }
    for (auto const& proc : processes) {
      ret.m_loc_state.insert(make_pair(proc.first, 0));
    }

    return ret;
  }

  bool operator==(state const& other)
  {
    return m_shared_state == other.m_shared_state
      && m_mutex_state == other.m_mutex_state
      && m_loc_state == other.m_loc_state;
  }

  string dump_string()
  {
    stringstream ss;
    ss << "State " << m_label << ":\n";
    ss << "\tSHARED_STATE:\n";
    for (auto const& p : m_shared_state) {
      ss << "\t\t" << p.first << " --> " << p.second << "\n";
    }

    ss << "\tMUTEX_STATE:\n";
    for (auto const& p : m_mutex_state) {
      ss << "\t\t" << p.first << " --> " << p.second.first << ", " << p.second.second << "\n";
    }

    ss << "\tLOC_STATE:\n";
    for (auto const& p : m_loc_state) {
      ss << "\t\t" << p.first << " --> " << p.second << "\n";
    }

    return ss.str();
  }

  // returns the next unique transaction to be executed by proc (may be enabled or disabled)
  instruction* get_process_next_transition(process* const& proc);
  unordered_set<process*> get_enabled_set(unordered_map<label, process*> const& all_procs);
  state get_next_state(instruction* const& ins);
};

class transition
{
private:
  state m_from_state;
  instruction* m_action;
  state m_to_state;
public:
  transition() {}

  transition(state const& from, instruction* const& action, state const& to)
    : m_from_state(from), m_to_state(to)
  {
    m_action = action;
  }
};

class dpor
{
private:
  concurrent_procs* m_data;

public:
  dpor() {}

  dpor(concurrent_procs* all_procs)
  {
    m_data = all_procs;
  }

  void test()
  {
    unordered_set<variable> shared_vars, mutex_vars;
    auto processes = m_data->get_processes();
    for (auto const& m : processes) {
      unordered_set_union(shared_vars, m.second->get_shared_vars());
      unordered_set_union(mutex_vars, m.second->get_mutex_vars());
    }
    state start = state::get_start_state(shared_vars, mutex_vars, m_data->get_processes());
    cout << start.dump_string() << endl;
  }
};

#endif
