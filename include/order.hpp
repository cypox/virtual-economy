#pragma once

#include "include/object.hpp"
#include "include/actor.hpp"

#include <set>


using order_list = std::set<order>;

enum order_type {
  BUY = 1,
  SELL = 2
};

class order {
public:
  order(order_type type, unsigned quantity, double strike, object_id obj, actor_id actor) : m_type(type), m_quantity(quantity), m_strike(strike), m_object_id(obj), m_actor_id(actor) { };

  bool operator<(const order& b)
  {
    if (m_type != b.get_type() || m_object_id != b.get_object_id())
    {
      throw std::logic_error("comparing two unequivalent orders");
    }
    return m_strike < b.get_strike();
  }

  order_type get_type() const { return m_type; };
  unsigned get_quantity() const { return m_quantity; };
  double get_strike() const { return m_strike; };
  object_id get_object_id() const { return m_object_id; };
  actor_id get_actor_id() const { return m_actor_id; };

private:
  order_type m_type;
  unsigned m_quantity;
  double m_strike;
  object_id m_object_id;
  actor_id m_actor_id;
};
