#pragma once

#include "order.hpp"
#include "object.hpp"

#include <map>


typedef unsigned actor_id;

template<class T> class world;

template<class logic>
class actor {
public:
  actor() = delete;

  actor(actor_id id, double cash, world<logic>* w) : m_id(id), m_cash(cash), m_logic(this, w) { };

  actor_id get_id() const
  {
    return m_id;
  }

  unsigned get_stock(object_id oid)
  {
    return m_stock[oid];
  }

  void step(order_list& order_list)
  {
    m_logic.take_decision(order_list);
  }

  void acquire(object_id obj, unsigned quantity)
  {
    m_stock[obj] += quantity;
  }

  void destroy(object_id obj, unsigned quantity)
  {
    if (m_stock[obj] > quantity)
    {
      m_stock[obj] -= quantity;
    }
  }

  order prepare_order(order_type type, unsigned quantity, double price, object_id obj)
  {
    if (type == order_type::SELL)
    {
      if (m_stock[obj] >= quantity)
      {
        m_stock[obj] -= quantity;
        m_reserve[obj] += quantity;
      }
      else
      {
        return order();
      }
    }
    else if (type == order_type::BUY)
    {
      double total_price = price * quantity;
      if (m_cash > total_price)
      {
        m_cash -= total_price;
        m_reserved_cash += total_price;
      }
      else
      {
        return order();
      }
    }
    return order(type, quantity, price, obj, m_id);
  }

  void execute_buy(object_id obj, unsigned quantity, double price)
  {
    double total_price = price * quantity;
    if (m_cash >= total_price)
    {
      m_reserved_cash -= total_price;
      m_stock[obj] += quantity;
    }
  }

  void execute_sell(object_id obj, unsigned quantity, double price)
  {
    double total_price = price * quantity;
    m_cash += total_price;
    m_reserve[obj] -= quantity;
  }

private:
  actor_id m_id;
  double m_cash;
  double m_reserved_cash;
  std::map<object_id, unsigned> m_stock;
  std::map<object_id, unsigned> m_reserve;
  logic m_logic;
};
