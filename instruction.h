#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <string>

using namespace std;

class instruction
{
private:
  string m_label;
public:
  instruction(string label) : m_label(label)
  { }
};

class assignment_instruction : instruction
{
private:
  string m_left;
  string m_right_var;
  int m_right_val;
  bool m_is_constant;

public:
  assignment_instruction(string label, string left, string right)
    : instruction(label), m_left(left), m_right_var(right)
  {
    m_is_constant = false;
  }

  assignment_instruction(string label, string left, int right)
    : instruction(label), m_left(left), m_right_val(right)
  {
    m_is_constant = true;
  }

};

class mutex_instuction : instruction
{
private:
  string m_mutex_var;
  bool m_is_acquire;

public:
  mutex_instuction(string label, string var, string mode)
    : instruction(label), m_mutex_var(var)
  {
    if (mode != "release" || mode != "acquire") {
      throw "Unidentified mutex mode";
    }
    m_is_acquire = mode == "acquire";
  }

};

#endif