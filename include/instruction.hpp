#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "util.hpp"

using namespace std;

enum instruction_type {
  assignment,
  mutex
};

using variable = string;
using label = string;

class process;
class instruction
{
protected:
  label m_label;
  label m_process_id;
  instruction_type m_type;
public:
  instruction()
  { }
  instruction(label label) : m_label(label)
  { }

  void set_label(label label) { m_label = label; }
  label get_instruction_label() { return m_label; }
  void set_process_id(label proc) { m_process_id = proc; }
  label get_process_id() { return m_process_id; }
  instruction_type get_instruction_type() { return m_type; }

  virtual string dump_string() const { return ""; }
};

// Represents instructions of the form x := e
class assignment_instruction : public instruction
{
private:
  variable m_left;
  variable m_right_var;
  int m_right_val;
  bool m_is_constant;

public:
  assignment_instruction(variable left, variable right)
    : m_left(left), m_right_var(right), m_is_constant(false)
  {
    m_type = assignment;
  }

  assignment_instruction(variable left, int right)
    : m_left(left), m_right_val(right), m_is_constant(true)
  {
    m_type = assignment;
  }

  variable get_lhs() { return m_left; }
  bool is_constant_assignment() { return m_is_constant; }
  int get_rhs_val() { return m_right_val; }
  variable get_rhs_var() { return m_right_var; }

  bool are_dependant(assignment_instruction* const& other)
  { 
    // Data races between 2 assignment instructions
    if (this->m_left == other->m_left) {
      return true;
    }
    if (this->m_is_constant && other->m_is_constant) {
      return false;
    } else if (this->m_is_constant) {
      return this->m_left == other->m_right_var;
    } else if (other->m_is_constant) {
      return other->m_left == this->m_right_var;
    } else {
      return this->m_left == other->m_right_var || other->m_left == this->m_right_var;
    }
    return false;
  }

  string dump_string() const override
  {
    stringstream ss;
    ss << m_label << ": " << m_left << " := ";
    if (m_is_constant) {
      ss << m_right_val;
    } else {
      ss << m_right_var;
    }
    return ss.str();
  }

};

// Represents acquire(), release() instructions
class mutex_instruction : public instruction
{
private:
  variable m_mutex_var;
  bool m_is_acquire;

public:
  mutex_instruction(variable var, bool mode)
    : m_mutex_var(var), m_is_acquire(mode)
  {
    m_type = mutex;
  }

  variable get_mutex_var() { return m_mutex_var; }
  bool is_acquire() {return m_is_acquire; }

  bool are_dependant(mutex_instruction* const& other)
  {
    // 2 acquires on the same shared variable are dependant
    return this->m_mutex_var == other->m_mutex_var
      && this->m_is_acquire && other->m_is_acquire;
  }

  string dump_string() const override
  {
    stringstream ss;
    ss << m_label << ": ";
    if (m_is_acquire) {
      ss << "acquire(";
    } else {
      ss << "release(";
    }
    ss << m_mutex_var << ")";
    return ss.str();
  }

};

class binary_label_relation
{
private:
  unordered_set<pair<label, label>, hash<pair<label, label>>> m_set;
public:
  binary_label_relation() : m_set()
  { }

  binary_label_relation(unordered_set<pair<label, label>> const& set)
    : m_set(set)
  { }

  binary_label_relation(vector<pair<label, label>> const& vec)
    : m_set(vec.begin(), vec.end())
  { }

  void add_pair(label l1, label l2) { m_set.insert(make_pair(l1, l2)); }
  bool exists(label l1, label l2) { return m_set.count(make_pair(l1, l2)) != 0; }

  int size() { return m_set.size(); }

  void relation_union(binary_label_relation const& other)
  {
    for (auto p : other.m_set) {
      m_set.insert(p);
    }
  }

  string dump_string() const
  {
    stringstream ss;
    ss << "{";
    int i = 0;
    for (auto const& p : m_set) {
      if (i != 0) {
        ss << ", ";
      }
      ss << "(" << p.first << ", " << p.second << ")";
      ++i;
    }
    ss << "}";
    return ss.str();
  }
};

#endif
