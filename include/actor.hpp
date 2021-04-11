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

  void create(object_id obj, unsigned quantity)
  {
    m_stock[obj] += quantity;
  }

  bool destroy(object_id obj, unsigned quantity)
  {
    if (m_stock[obj] > quantity)
    {
      m_stock[obj] -= quantity;
    }
    return m_stock[obj] == 0;
  }

  order make_order(order_type type, unsigned quantity, double price, object_id obj)
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

  bool execute_buy(object_id obj, unsigned quantity, double price)
  {
    double total_price = price * quantity;
    if (m_cash >= total_price)
    {
      m_reserved_cash -= total_price;
      m_stock[obj] += quantity;
      return true;
    }
    return false;
  }

  bool execute_sell(object_id obj, unsigned quantity, double price)
  {
    if (m_reserve[obj] >= quantity)
    {
      double total_price = price * quantity;
      m_cash += total_price;
      m_reserve[obj] -= quantity;
      return true;
    }
    throw std::runtime_error("cannot execute sell");
  }

  void render() const
  {
    printf("%d have %.2f liquidity and %.2f in reserve with the following stock:\n", m_id, m_cash, m_reserved_cash);
    for(auto s : m_stock)
    {
      printf("  %d with a quantity of %d in stock\n", s.first, s.second);
    }
    for(auto s : m_reserve)
    {
      printf("  %d with a quantity of %d in reserve\n", s.first, s.second);
    }
  }

private:
  actor_id m_id;
  double m_cash;
  double m_reserved_cash;
  std::map<object_id, unsigned> m_stock;
  std::map<object_id, unsigned> m_reserve;
  logic m_logic;
};
