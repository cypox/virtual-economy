#pragma once

#include "include/order.hpp"
#include "include/object.hpp"

#include <map>


using actor_id = unsigned;

class actor {
public:
  actor() = delete;

  actor(actor_id id, double cash) : m_id(id), m_cash(cash) { };

  actor_id get_id() const
  {
    return m_id;
  }

  void step(order_list& order_list)
  {
    order test = place_order(0, 1, 10, order_type::BUY);
    order_list.insert(test);
  }

  void acquire(object_id obj, unsigned quantity)
  {
    m_stock[obj] += quantity;
  }

  void destruct(object_id obj, unsigned quantity)
  {
    if (m_stock[obj] > quantity)
    {
      m_stock[obj] -= quantity;
    }
  }

  order place_order(object_id obj, unsigned quantity, double price, order_type type)
  {
    if (type == order_type::SELL)
    {
      if (m_stock[obj] >= quantity)
      {
        m_stock[obj] -= quantity;
        m_reserve[obj] += quantity;
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
};
