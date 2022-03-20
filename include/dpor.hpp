#ifndef DPOR_HPP
#define DPOR_HPP

#include "program.hpp"
#include <assert.h>
#include <fstream>

using namespace std;

enum mutex_status {
  unlocked,
  locked
};

string enum_mutex_to_string(mutex_status m);

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

  unordered_set<process*> m_backtrack_set;
  unordered_set<process*> m_done_set;
  unordered_set<process*> m_sleep_set;
public:
  state()
  { }

  state(label label) : m_label(label)
  { }

  state(state const& other)
    : m_shared_state(other.m_shared_state),
      m_mutex_state(other.m_mutex_state),
      m_loc_state(other.m_loc_state)
  { }
  
  label get_label() { return m_label; }
  void set_label(string label) { m_label = label; }
  unordered_set<process*> get_backtrack_set() { return m_backtrack_set; }
  void set_backtrack_set(unordered_set<process*> bs) { m_backtrack_set = bs; }
  void add_to_backtrack_set(process* proc) { m_backtrack_set.insert(proc); }

  unordered_set<process*> get_done_set() { return m_done_set; }
  void set_done_set(unordered_set<process*> bs) { m_done_set = bs; }
  void add_to_done_set(process* proc) { m_done_set.insert(proc); }

  unordered_set<process*> get_sleep_set() { return m_sleep_set; }
  void add_to_sleep_set(process* proc) { m_sleep_set.insert(proc); }

  // Returns the start state with all shared variables
  // initialized to zero, all mutex variables unlocked,
  // and pc's for all processes to zero
  static state* get_start_state(
    unordered_set<variable> const& shared_vars,
    unordered_set<variable> const& mutex_vars,
    unordered_map<label, process*> const& processes
  )
  {
    auto ret = new state("0");
    for (auto const& var : shared_vars) {
      ret->m_shared_state.insert(make_pair(var, 0));
    }
    for (auto const& mutex : mutex_vars) {
      ret->m_mutex_state.insert(make_pair(mutex, make_pair(unlocked, "")));
    }
    for (auto const& proc : processes) {
      ret->m_loc_state.insert(make_pair(proc.first, 0));
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
      ss << "\t\t" << p.first << " --> " << enum_mutex_to_string(p.second.first) << ", " << p.second.second << "\n";
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
  state* get_next_state(instruction* const& ins);
};

class transition
{
private:
  state* m_from_state;
  instruction* m_action;
  state* m_to_state;
public:
  transition() {}

  transition(state* const& from, instruction* const& action, state* const& to)
    : m_from_state(from), m_to_state(to)
  {
    m_action = action;
  }

  state* get_from_state() const { return m_from_state; }
  state* get_to_state() const { return m_to_state; }
  instruction* get_action() const { return m_action; }
};

// pid --> N mapping
using clock_vector = unordered_map<label, int>;

class clock_vectors
{
private:
  unordered_map<label, clock_vector> m_process_map;
  unordered_map<int, clock_vector> m_transition_map;
  vector<label> m_ids;

public:
  clock_vectors() {}
  clock_vectors(vector<label> ids) : m_ids(ids)
  { }

  clock_vector empty_clock_vector()
  {
    clock_vector ret;
    for (auto const& p : m_ids) {
      ret[p] = 0;
    }
    return ret;
  }
  
  clock_vector get_clock_vector(label l)
  {
    if (m_process_map.count(l)) {
      return m_process_map[l];
    }
    clock_vector e = empty_clock_vector();
    m_process_map[l] = e;
    return e;
  }

  clock_vector get_clock_vector(int n)
  {
    if (m_transition_map.count(n)) {
      return m_transition_map[n];
    }
    clock_vector e = empty_clock_vector();
    m_transition_map[n] = e;
    return e;
  }

  void set_clock_vector(label l, clock_vector cv) { m_process_map[l] = cv; }
  void set_clock_vector(int n, clock_vector cv) { m_transition_map[n] = cv; }

  clock_vector clock_vector_max(clock_vector cv1, clock_vector cv2)
  {
    clock_vector ret = empty_clock_vector();
    for (auto const& p : cv1) {
      ret[p.first] = max(p.second, cv2[p.first]);
    }

    return ret;
  }

};

class dpor
{
private:
  string m_input_file;
  string m_dot_file;
  concurrent_procs* m_data;
  vector<state*> m_states;
  vector<transition> m_transitions;
  state* m_start_state;
  int m_executions;

public:
  dpor() {}

  dpor(concurrent_procs* all_procs, string input, string dot_file)
    : m_input_file(input), m_dot_file(dot_file)
  {
    m_data = all_procs;
    m_executions = 0;
  }

  void initialize_with_start_state()
  {
    unordered_set<variable> shared_vars, mutex_vars;
    auto processes = m_data->get_processes();
    for (auto const& m : processes) {
      unordered_set_union(shared_vars, m.second->get_shared_vars());
      unordered_set_union(mutex_vars, m.second->get_mutex_vars());
    }
    state* start = state::get_start_state(shared_vars, mutex_vars, m_data->get_processes());
    // cout << start->dump_string() << endl;
    m_states.push_back(start);
    m_start_state = start;
  }

  state* last_transition_sequence_state(vector<transition> const& S)
  {
    if (S.size() == 0) {
      return m_start_state;
    }
    return S[S.size()-1].get_to_state();
  }

  state* find_state(state* const& s);
  void dynamic_por();
  void explore(vector<transition> &stack, clock_vectors C);

  string get_stats()
  {
    stringstream ss;
    ss << "NUM_STATES = " << m_states.size() << "\n";
    ss << "NUM_TRANSITIONS = " << m_transitions.size() << "\n";
    ss << "NUM_EXECUTIONS = " << m_executions << "\n";

    return ss.str();
  }

  void print_to_dot_format();
};

#endif
