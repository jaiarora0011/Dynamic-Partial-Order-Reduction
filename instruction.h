#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <sstream>
#include "util.h"

using namespace std;

class instruction
{
protected:
  string m_label;
public:
  instruction()
  { }
  instruction(string label) : m_label(label)
  { }

  void set_label(string label) { m_label = label; }

  string get_instruction_label() { return m_label; }

  virtual string dump_string() const { return ""; }
};

class assignment_instruction : public instruction
{
private:
  string m_left;
  string m_right_var;
  int m_right_val;
  bool m_is_constant;

public:
  assignment_instruction(string left, string right)
    : m_left(left), m_right_var(right), m_is_constant(false)
  { }

  assignment_instruction(string left, int right)
    : m_left(left), m_right_val(right), m_is_constant(true)
  { }

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

class mutex_instruction : public instruction
{
private:
  string m_mutex_var;
  bool m_is_acquire;

public:
  mutex_instruction(string var, bool mode)
    : m_mutex_var(var), m_is_acquire(mode)
  { }

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
  unordered_set<pair<string, string>, hash<pair<string, string>>> m_set;
public:
  binary_label_relation() : m_set()
  { }

  binary_label_relation(unordered_set<pair<string, string>> const& set)
    : m_set(set)
  { }

  binary_label_relation(vector<pair<string, string>> const& vec)
    : m_set(vec.begin(), vec.end())
  { }

  void add_pair(string l1, string l2)
  {
    m_set.insert(make_pair(l1, l2));
  }

  void relation_union(binary_label_relation* const& other)
  {
    for (auto p : other->m_set) {
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