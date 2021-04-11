#pragma once

#include "object.hpp"

#include <set>


typedef unsigned object_id;
typedef unsigned actor_id;

enum order_type {
  BUY = 1,
  SELL = 2
};

class order {
public:
  order() : m_valid(false) { };
  order(order_type type, unsigned quantity, double strike, object_id obj, actor_id actor) : m_type(type), m_quantity(quantity), m_strike(strike), m_object_id(obj), m_actor_id(actor), m_valid(true) { };

  bool operator<(const order& lhs) const
  {
    if (m_strike == lhs.get_strike())
    {
      return m_actor_id < lhs.get_actor_id();
    }
    return m_strike < lhs.get_strike();
  }

  void render()
  {
    printf("%d wants to %s %d of %d at %.2f\n", m_actor_id, m_type==order_type::BUY?"BUY":"SELL", m_quantity, m_object_id, m_strike);
  }

  order_type get_type() const { return m_type; };
  unsigned get_quantity() const { return m_quantity; };
  double get_strike() const { return m_strike; };
  object_id get_object_id() const { return m_object_id; };
  actor_id get_actor_id() const { return m_actor_id; };
  bool is_valid() const { return m_valid; };

private:
  order_type m_type;
  unsigned m_quantity;
  double m_strike;
  object_id m_object_id;
  actor_id m_actor_id;
  bool m_valid;
};

typedef std::set<order> order_list;
