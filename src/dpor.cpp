#include "dpor.hpp"

using namespace std;

string enum_mutex_to_string(mutex_status m)
{
  if (m == unlocked) {
    return "unlocked";
  } else {
    return "locked";
  }
}

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
  if (i1 == NULL || i2 == NULL) {
    return false;
  }
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

bool
may_be_coenabled(instruction* i1, instruction* i2)
{ 
  if (i1->get_process_id() == i2->get_process_id()) {
    return false;
  }
  if (i1->get_instruction_type() == mutex) {
    auto a1 = dynamic_cast<mutex_instruction*>(i1);
    auto a2 = dynamic_cast<mutex_instruction*>(i2);
    assert(a1);
    assert(a2);
    return !(a1->get_mutex_var() == a2->get_mutex_var()
      && a1->is_acquire() != a2->is_acquire());
  }
  return true;
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

instruction*
state::get_process_next_transition(process* const& proc)
{
  int pc = m_loc_state[proc->get_process_label()];
  if (pc >= proc->get_instruction_list().size()) {
    return NULL;
  } else {
    return proc->get_instruction_list()[pc];
  }
}

unordered_set<process*>
state::get_enabled_set(unordered_map<label, process*> const& all_procs)
{
  unordered_set<process*> ret;
  for (auto const& p : all_procs) {
    auto ins = get_process_next_transition(p.second);
    if (ins == NULL) {
      continue;
    }
    // A mutex instruction may get blocked
    if (ins->get_instruction_type() == mutex) {
      auto mut = dynamic_cast<mutex_instruction*>(ins);
      assert(mut);
      auto status = m_mutex_state[mut->get_mutex_var()];
      if (status.first == locked) {
        if (status.second != p.first) {
          // Another process cannot operate on a lock owned by a different process
          continue;
        } else if (mut->is_acquire()) {
          // cannot call acquire on an already acquired lock
          continue;
        }
      } else {
        // cannot call release on an unlocked mutex
        if (!mut->is_acquire()) {
          continue;
        }
      }
    }
    ret.insert(p.second);
  }

  return ret;
}

// Assume that the instruction is enabled at the current state
state*
state::get_next_state(instruction* const& ins)
{
  assert(ins);
  state* next = new state(*this);
  if (ins->get_instruction_type() == mutex) {
    auto mut = dynamic_cast<mutex_instruction*>(ins);
    assert(mut);
    auto status = next->m_mutex_state[mut->get_mutex_var()];
    if (status.first == locked) {
      assert(status.second == mut->get_process_id());
      assert(!mut->is_acquire());
      next->m_mutex_state[mut->get_mutex_var()] = make_pair(unlocked, "");
    } else {
      assert(mut->is_acquire());
      next->m_mutex_state[mut->get_mutex_var()] = make_pair(locked, mut->get_process_id());
    }
  } else {
    auto assign = dynamic_cast<assignment_instruction*>(ins);
    assert(assign);
    if (assign->is_constant_assignment()) {
      next->m_shared_state[assign->get_lhs()] = assign->get_rhs_val();
    } else {
      int read_val = next->m_shared_state.at(assign->get_rhs_var());
      next->m_shared_state[assign->get_lhs()] = read_val;
    }
  }
  // Increment the pc of the executing process
  next->m_loc_state[ins->get_process_id()]++;
  // next->m_label = this->m_label + "." + ins->get_instruction_label();

  return next;
}

state*
dpor::find_state(state* const& s)
{
  for (auto const& state : m_states) {
    if (*state == *s) {
      return state;
    }
  }
  m_states.push_back(s);
  s->set_label(to_string(m_states.size()-1));
  return s;
}

void
dpor::explore(vector<transition> &stack, clock_vectors C)
{
  auto last_state = this->last_transition_sequence_state(stack);
  
  for (auto const& p : m_data->get_processes()) {
    auto next_s_p = last_state->get_process_next_transition(p.second);
    if (next_s_p == NULL) {
      continue;
    }
    bool found = false;
    int index;
    for (int i = stack.size() - 1; i >= 0; i--) {
      auto ins = stack[i].get_action();
      if (m_data->get_dependant_set().exists(ins->get_instruction_label(), next_s_p->get_instruction_label())
        && may_be_coenabled(ins, next_s_p) && i + 1 > C.get_clock_vector(p.first)[ins->get_process_id()]) {
        found = true;
        index = i;
        break;
      }
    }
    if (found) {
      auto ins = stack[index].get_action();
      auto pre_s_i = stack[index].get_from_state();
      auto enabled_set = pre_s_i->get_enabled_set(m_data->get_processes());
      if (enabled_set.count(p.second)) {
        // cout << "On state " << last_state->get_label() << ", Adding " << p.first << " to backtrack set of " << pre_s_i->get_label() << endl;
        pre_s_i->add_to_backtrack_set(p.second);
        pre_s_i->add_to_sleep_set(m_data->get_processes()[ins->get_process_id()]);
        // cout << "Adding " << ins->get_process_id() << " to sleep set of " << pre_s_i->get_label() << endl;
      } else {
        for (auto const& en : enabled_set) {
          pre_s_i->add_to_backtrack_set(en);
        }
      }
    }
  }

  auto enabled_set = last_state->get_enabled_set(m_data->get_processes());
  unordered_set_difference(enabled_set, last_state->get_sleep_set());

  if (enabled_set.size()) {
    auto proc = *enabled_set.begin();
    unordered_set<process*> bs;
    bs.insert(proc);
    last_state->set_backtrack_set(bs);
    unordered_set<process*> done, sleep;

    while (true) {
      bs = last_state->get_backtrack_set();
      done = last_state->get_done_set();
      sleep = last_state->get_sleep_set();
      // cout << "done Set at " << last_state->get_label() << endl;
      // for (auto const& sleep_proc : done) {
      //   cout << "\t" << sleep_proc->get_process_label() << endl;
      // }
      // cout << "sleep Set at " << last_state->get_label() << endl;
      // for (auto const& sleep_proc : sleep) {
      //   cout << "\t" << sleep_proc->get_process_label() << endl;
      // }
      unordered_set_difference(bs, done);
      if (!bs.size()) {
        break;
      }
      auto proc = *bs.begin();
      // cout <<  "Chosen Process from enable set at " << last_state->get_label() << " = " << proc->get_process_label() << endl;
      // cout << "BackTrack set size " <<  bs.size() << endl;
      // cout << "At state " << last_state->get_label() << ", chosen proc = " << proc->get_process_label() << endl;
      last_state->add_to_done_set(proc);
      if (sleep.count(proc)) {
        continue;
      }
      auto next_s_p = last_state->get_process_next_transition(proc);
      auto empty_cv = C.empty_clock_vector();
      for (int i = 0; i < stack.size(); ++i) {
        auto ins = stack[i].get_action();
        if (m_data->get_dependant_set().exists(ins->get_instruction_label(), next_s_p->get_instruction_label())) {
          empty_cv = C.clock_vector_max(empty_cv, C.get_clock_vector(i+1));
        }
      }
      auto next_state = last_state->get_next_state(next_s_p);
      next_state = find_state(next_state);
      for (auto const& sleep_proc : sleep) {
        auto ins = last_state->get_process_next_transition(sleep_proc);
        if (are_instructions_dependant(ins, next_s_p)) {
          continue;
        }
        // cout << "Adding " << sleep_proc->get_process_label() << " to sleep set of " << next_state->get_label() << endl;
        next_state->add_to_sleep_set(sleep_proc);
      }
      transition new_transition(last_state, next_s_p, next_state);
      m_transitions.push_back(new_transition);
      stack.push_back(new_transition);
      empty_cv[proc->get_process_label()] = stack.size();
      C.set_clock_vector(proc->get_process_label(), empty_cv);
      C.set_clock_vector(stack.size(), empty_cv);
      explore(stack, C);
      stack.pop_back();
    }
  } else {
    m_executions++;
  }

}

void
dpor::dynamic_por()
{ 
  vector<label> ids;
  for (auto const& p : m_data->get_processes()) {
    ids.push_back(p.first);
  }
  clock_vectors C(ids);
  this->initialize_with_start_state();
  vector<transition> stack;
  explore(stack, C);
}

void
dpor::print_to_dot_format()
{
  ofstream out(m_dot_file);

  out << "digraph{\n";
  out << "\tnodesep = 0.5;\n";
  out << "\tranksep = 0.35;\n";

  for (int i = 0; i < m_states.size(); ++i) {
    out << "\t" << i;
    out << "\n";
  }
  // out << "\tsubgraph dir\n";
  // out << "\t{\n";

  for (auto const& t : m_transitions) {
    out << "\t" << t.get_from_state()->get_label() << " -> " << t.get_to_state()->get_label() << " [label=\"" << t.get_action()->dump_string() << "\"];\n";
  }

  // out << "\t}\n";
  out << "}";
  out.close();
}