#pragma once

#include "order.hpp"
#include "transaction.hpp"

#include <cmath>
#include <queue>
#include <map>
#include <list>
#include <set>


class market {
public:
  market(unsigned execution_rate = 10) : m_execution_rate(execution_rate) { }

  void step(const order_list& orders, std::queue<transaction>& tr_list, bool end_trading_day = false)
  {
    if (end_trading_day)
    {
      cancel_all_orders();
      return;
    }

    for (order ord : orders)
    {
      m_order_queue.push(ord);
    }

    if (!m_order_queue.empty())
    {
      for (int i = 0 ; i < m_execution_rate && !m_order_queue.empty() ; ++ i)
      {
        order to_satisfy = m_order_queue.front();
        // TODO: check if the order is not satisfied before popping
        // if the order cannot be satisfied, it should be returned to the queue
        m_order_queue.pop();
        tr_list.push(process_single_order(to_satisfy));
      }
    }
  }

  void cancel_all_orders()
  {
    while(!m_order_queue.empty())
    {
      m_order_queue.pop();
    }
    m_buy_orders.clear();
    m_sell_orders.clear();
  }

  transaction self_settle(const order& to_cancel)
  {
    order_type settle_type = (to_cancel.get_type() == order_type::BUY) ? order_type::SELL : order_type::BUY;
    order second(settle_type, to_cancel.get_quantity(), to_cancel.get_strike(), to_cancel.get_object_id(), to_cancel.get_actor_id());
    transaction tr = execute_order(to_cancel, second);
    return tr;
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
    // TODO: the price is not second_order.get_strike but the price asked for by the seller (not buying for more than I can pay)
    return transaction(buyer, seller, second_order.get_object_id(), second_order.get_quantity(), second_order.get_strike());
  }

  transaction process_single_order(const order& to_satisfy)
  {
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
        // TODO: use a priority queue or a more efficient structure for retrieving the min
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
        // TODO: use a priority queue or a more efficient structure for retrieving the min
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

  double get_lowest_offer(object_id oid) const
  {
    double lowest_offer = INFINITY;
    if (m_sell_orders.count(oid))
    {
      for(const order& o : m_sell_orders.at(oid))
      {
        if (o.get_strike() < lowest_offer)
        {
          lowest_offer = o.get_strike();
        }
      }
    }
    return lowest_offer;
  }

  double get_highest_bid(object_id oid) const
  {
    double highest_bid = 0.f;
    if (m_buy_orders.count(oid))
    {
      for(const order& o : m_buy_orders.at(oid))
      {
        if (o.get_strike() > highest_bid)
        {
          highest_bid = o.get_strike();
        }
      }
    }
    return highest_bid;
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

  const std::queue<order>& get_order_queue() const { return m_order_queue; };
  const std::map<object_id, order_list>& get_buy_order_list() const { return m_buy_orders; };
  const std::map<object_id, order_list>& get_sell_order_list() const { return m_sell_orders; };

private:
  std::queue<order> m_order_queue;
  std::map<object_id, order_list> m_buy_orders;
  std::map<object_id, order_list> m_sell_orders;
  unsigned m_execution_rate;
};
