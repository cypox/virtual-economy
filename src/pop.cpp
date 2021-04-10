#include "include/pop.hpp"

#include <iostream>


pop::pop()
{
  m_size = 0;
}

void pop::feed()
{
  std::cout << "eating" << std::endl;
}

void pop::work()
{
  std::cout << "working" << std::endl;
}

void pop::trade()
{
  std::cout << "trading" << std::endl;
}

unsigned pop::get_size() const
{
  return m_size;
}

void pop::set_size(unsigned size)
{
  m_size = size;
}
