#include "include/object.hpp"

#include <iostream>


object::object()
{
  m_price = 0.0;
}

void object::feed()
{
  std::cout << "eating" << std::endl;
}

double object::get_price() const
{
  return m_price;
}

void object::set_price(double price)
{
  m_price = price;
}
