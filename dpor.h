#ifndef DPOR_H
#define DPOR_H

#include "program.h"
#include <assert.h>

using namespace std;

enum mutex_status {
  unlocked,
  locked
};

string mutex_to_string(mutex_status m)
{
  if ( m == unlocked) {
    return "unlocked";
  } else {
    return "locked";
  }
}

class state
{
private:
  // var --> value mapping
  unordered_map<variable, int> m_shared_state;
  // lock --> (status, owner) mapping
  unordered_map<variable, pair<mutex_status, label>> m_mutex_state;
  // process_id --> pc (to be executed) mapping
  unordered_map<label, int> m_loc_state;
public:
  state()
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
    state ret;
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
    ss << "SHARED_STATE:\n";
    for (auto const& p : m_shared_state) {
      ss << "\t" << p.first << " --> " << p.second << "\n";
    }

    ss << "MUTEX_STATE:\n";
    for (auto const& p : m_mutex_state) {
      ss << "\t" << p.first << " --> " << mutex_to_string(p.second.first) << ", " << p.second.first << "\n";
    }

    ss << "LOC_STATE:\n";
    for (auto const& p : m_shared_state) {
      ss << "\t" << p.first << " --> " << p.second << "\n";
    }

    return ss.str();
  }

  // returns the next unique transaction to be executed by proc (may be enabled or disabled)
  instruction* get_process_next_transition(process* const& proc);
  unordered_set<process*> get_enabled_set(unordered_map<label, process*> const& all_procs);
};

#endif