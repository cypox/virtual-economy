#pragma once

#include "order.hpp"
#include "transaction.hpp"

#include <queue>
#include <map>
#include <list>
#include <set>


class market {
public:
  market() { }

  void step(const order_list& orders, transaction& tr)
  {
    for (order ord : orders)
    {
      place_order(ord);
    }

    if (!m_order_queue.empty())
    {
      tr = process_single_order();
    }
  }

  void place_order(const order o)
  {
    m_order_queue.push(o);
  }

  transaction execute_order(const order& to_satisfy, order second_order)
  {
    if (to_satisfy.get_quantity() != second_order.get_quantity())
    {
      throw std::runtime_error("different quantities settlments are not implemented yet");
    }
    actor_id seller;
    actor_id buyer;
    if (to_satisfy.get_type() == order_type::BUY)
    {
      seller = second_order.get_actor_id();
      buyer = to_satisfy.get_actor_id();
    }
    else if (to_satisfy.get_type() == order_type::SELL)
    {
      seller = to_satisfy.get_actor_id();
      buyer = second_order.get_actor_id();
    }
    return transaction(buyer, seller, to_satisfy.get_object_id(), to_satisfy.get_quantity(), second_order.get_strike());
  }

  transaction process_single_order()
  {
    order to_satisfy = m_order_queue.front();
    m_order_queue.pop();

    if (to_satisfy.get_quantity() != 1)
    {
      throw std::runtime_error("orders with quantity!=1 are not yet implemented");
    }

    if (to_satisfy.get_type() == order_type::BUY)
    {
      order_list& market_orders = m_sell_orders[to_satisfy.get_object_id()];
      auto it = market_orders.begin();
      while(it != market_orders.end())
      {
        const order current_order = *it;
        if (current_order.get_strike() <= to_satisfy.get_strike())
        {
          actor_id second = current_order.get_actor_id();
          market_orders.erase(it);
          return execute_order(to_satisfy, current_order);
        }
        it ++;
      }
      m_buy_orders[to_satisfy.get_object_id()].insert(to_satisfy);
    }
    else if (to_satisfy.get_type() == order_type::SELL)
    {
      order_list& market_orders = m_buy_orders[to_satisfy.get_object_id()];
      auto it = market_orders.rbegin();
      while(it != market_orders.rend())
      {
        const order current_order = *it;
        if (current_order.get_strike() >= to_satisfy.get_strike())
        {
          actor_id second = current_order.get_actor_id();
          market_orders.erase(std::next(it).base());
          return execute_order(to_satisfy, current_order);
        }
        it ++;
      }
      m_sell_orders[to_satisfy.get_object_id()].insert(to_satisfy);
    }
    return transaction();
  }

  void render()
  {
    for (auto ol : m_buy_orders)
    {
      for (auto o : ol.second)
      {
        o.render();
      }
    }
    for (auto ol : m_sell_orders)
    {
      for (auto o : ol.second)
      {
        o.render();
      }
    }
  }

private:
  std::queue<order> m_order_queue;
  std::map<object_id, order_list> m_buy_orders;
  std::map<object_id, order_list> m_sell_orders;
};
