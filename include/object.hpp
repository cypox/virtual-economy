#pragma once

#include <cstdio>


using object_id = unsigned;

class object {
public:
  object() = delete;

  object(object_id id, double price) : m_id(id), m_price(price) { };

  double get_price() const
  {
    return m_price;
  }

  void set_price(double price)
  {
    m_price = price;
  }

  object_id get_id() const
  {
    return m_id;
  }

  void render() const
  {
    printf("%d has a current price of %f\n", m_id, m_price);
  }

private:
  object_id m_id;
  double m_price;
};
