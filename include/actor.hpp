#pragma once

#include "defines.hpp"
#include "include/transaction.hpp"
#include "include/order.hpp"
#include "include/object.hpp"

#include <map>
#include <list>


typedef unsigned actor_id;

template<class T> class world;

template<class logic>
class actor {
public:
  actor() = delete;

  actor(actor_id id, double cash, world<logic>* w) : m_id(id), m_cash(cash), m_storage_size(500), m_remaining_storage(m_storage_size), m_logic(this, w) { };

  actor_id get_id() const
  {
    return m_id;
  }

  double get_remaining_storage() const
  {
    return m_remaining_storage;
  }

  double get_total_cash() const
  {
    return m_cash + m_reserved_cash;
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

  order_list step()
  {
    order_list ol;
    m_logic.take_decision(ol);

    check_validity();
    return ol;
  }

  void check_validity() const
  {
    bool valid = m_remaining_storage >= 0;
    valid &= m_reserved_cash >= 0;
    valid &= m_cash >= 0;
    
    size_t total_storage = m_remaining_storage;
    size_t reserve_storage = 0;
    for (auto& item : m_stock)
    {
      valid &= item.second >= 0;
      total_storage += item.second;
    }

    for (auto& item : m_reserve)
    {
      valid &= item.second >= 0;
      reserve_storage += item.second;
    }

    valid &= (total_storage + reserve_storage) == m_storage_size;

    double total_buys = m_reserved_cash;
    size_t total_sells_size = 0;
    for (auto& ord : m_orders_copy)
    {
      if (ord.get_type() == order_type::BUY)
      {
        total_buys -= ord.get_strike();
      }
      else if (ord.get_type() == order_type::SELL)
      {
        total_sells_size += ord.get_quantity();
      }
    }
    //valid &= total_buys < 1.e-9;
    //valid &= total_sells_size == reserve_storage;

    if (!valid)
    {
      throw std::runtime_error("actor is not valid");
    }
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
    order ord(type, quantity, price, obj, m_id);
    m_orders_copy.push_back(ord);
    return ord;
  }

  void cancel_all_orders()
  {
    for (auto l : m_stock)
    {
      m_stock[l.first] += m_reserve[l.first];
      m_reserve[l.first] = 0;
    }
    m_cash += m_reserved_cash;
    m_reserved_cash = 0;
    while(!m_orders_copy.empty())
    {
      //order o = m_orders_copy.front();
      //cancel_order(o);
      m_orders_copy.pop_front();
    }
  }

  void cancel_order(order& o)
  {
    object_id obj = o.get_object_id();
    order_type type = o.get_type();
    double price = o.get_strike();
    unsigned quantity = o.get_quantity();
    if (type == order_type::SELL)
    {
      m_stock[obj] += quantity;
      m_reserve[obj] -= quantity;
    }
    else if (type == order_type::BUY)
    {
      double total_price = price * quantity;
      m_cash += total_price;
      m_reserved_cash -= total_price;
    }
    if (m_reserved_cash < 0 or m_reserve[obj] < 0)
    {
      throw std::runtime_error("order cancelling went wrong");
    }
  }

  bool execute_transaction(const transaction& t)
  {
    if (m_id == t.get_buyer())
    {
      return execute_buy(t.get_object(), t.get_quantity(), t.get_price());
    }
    else if (m_id == t.get_seller())
    {
      return execute_sell(t.get_object(), t.get_quantity(), t.get_price());
    }
    return false;
  }

  bool execute_buy(object_id obj, unsigned quantity, double price)
  {
    double total_price = price * quantity;
    if (m_remaining_storage <= quantity)
    {
      quantity = m_remaining_storage;
    }
    m_remaining_storage -= quantity;
    m_reserved_cash -= total_price;
    if (std::abs(m_reserved_cash) < EPSILON) m_reserved_cash = 0;
    m_stock[obj] += quantity;
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
  size_t m_storage_size;
  size_t m_remaining_storage;
  std::map<object_id, int> m_stock;
  std::map<object_id, int> m_reserve;
  std::list<order> m_orders_copy;
  logic m_logic;
};
