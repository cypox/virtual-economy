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

  actor(actor_id id, double cash, world<logic>* w) : m_id(id), m_cash(cash), m_remaining_storage(500), m_logic(this, w) { };

  actor_id get_id() const
  {
    return m_id;
  }

  double get_remaining_storage() const
  {
    return m_remaining_storage;
  }

  double get_cash() const
  {
    return m_cash;
  }

  double get_reserved_cash() const
  {
    return m_reserved_cash;
  }

  unsigned get_stock(object_id oid) const
  {
    if (m_stock.count(oid))
      return m_stock.at(oid);
    else
      return 0;
  }

  unsigned get_reserve(object_id oid) const
  {
    if (m_reserve.count(oid))
      return m_reserve.at(oid);
    else
      return 0;
  }

  void step(order_list& order_list)
  {
    m_logic.take_decision(order_list);
  }

  void create(object_id obj, unsigned quantity)
  {
    if (m_remaining_storage < quantity)
    {
      quantity = m_remaining_storage;
    }
    m_stock[obj] += quantity;
    m_remaining_storage -= quantity;
  }

  bool destroy(object_id obj, unsigned quantity)
  {
    if (m_stock[obj] < quantity)
    {
      quantity = m_stock[obj];
    }
    m_stock[obj] -= quantity;
    m_remaining_storage += quantity;
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
    m_reserved_cash -= total_price;
    m_stock[obj] += quantity;
    m_remaining_storage -= quantity;
    return m_reserved_cash >= 0;
  }

  bool execute_sell(object_id obj, unsigned quantity, double price)
  {
    double total_price = price * quantity;
    m_cash += total_price;
    m_reserve[obj] -= quantity;
    m_remaining_storage += quantity;
    return m_reserve[obj] >= 0;
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
  size_t m_remaining_storage;
  std::map<object_id, int> m_stock;
  std::map<object_id, int> m_reserve;
  logic m_logic;
};
