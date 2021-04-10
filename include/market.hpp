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

    tr = process_single_order();
  }

  void place_order(const order o)
  {
    m_order_queue.push(o);
  }

  transaction execute_order(const order& to_satisfy, actor_id second)
  {
    actor_id seller;
    actor_id buyer;
    if (to_satisfy.get_type() == order_type::SELL)
    {
      seller = second;
      buyer = to_satisfy.get_actor_id();
    }
    else if (to_satisfy.get_type() == order_type::SELL)
    {
      seller = to_satisfy.get_actor_id();
      buyer = second;
    }
    return transaction(buyer, seller, to_satisfy.get_object_id(), to_satisfy.get_quantity(), to_satisfy.get_strike());
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
        const order& current_order = *it;
        if (current_order.get_strike() <= to_satisfy.get_strike())
        {
          actor_id second = current_order.get_actor_id();
          m_sell_orders[to_satisfy.get_object_id()].erase(it);
          return execute_order(to_satisfy, second);
        }
      }
      m_buy_orders[to_satisfy.get_object_id()].insert(to_satisfy);
    }
    else if (to_satisfy.get_type() == order_type::SELL)
    {
      order_list& market_orders = m_buy_orders[to_satisfy.get_object_id()];
      auto it = market_orders.rbegin();
      while(it != market_orders.rend())
      {
        const order& current_order = *it;
        if (current_order.get_strike() >= to_satisfy.get_strike())
        {
          actor_id second = current_order.get_actor_id();
          m_buy_orders[to_satisfy.get_object_id()].erase(std::next(it).base());
          return execute_order(to_satisfy, second);
        }
      }
      m_sell_orders[to_satisfy.get_object_id()].insert(to_satisfy);
    }
    return transaction();
  }

private:
  std::queue<order> m_order_queue;
  std::map<object_id, order_list> m_buy_orders;
  std::map<object_id, order_list> m_sell_orders;
};
