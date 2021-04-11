#pragma once

#include "order.hpp"
#include "transaction.hpp"

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
      cancel_all_orders(orders, tr_list);
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
        m_order_queue.pop();
        tr_list.push(process_single_order(to_satisfy));
      }
    }
  }

  void cancel_all_orders(const order_list& orders, std::queue<transaction>& tr_list)
  {
    throw std::runtime_error("function is bugged");
    for (order ord : orders)
    {
      tr_list.push(self_settle(ord));
    }

    while(!m_order_queue.empty())
    {
      order top_order = m_order_queue.front();
      m_order_queue.pop();
      tr_list.push(self_settle(top_order));
    }

    for(auto s : m_buy_orders)
    {
      for (auto o : s.second)
      {
        tr_list.push(self_settle(o));
      }
    }
    m_buy_orders.clear();

    for(auto s : m_sell_orders)
    {
      for (auto o : s.second)
      {
        tr_list.push(self_settle(o));
      }
    }
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

  const std::queue<order>& get_order_queue() const { return m_order_queue; };
  const std::map<object_id, order_list>& get_buy_order_list() const { return m_buy_orders; };
  const std::map<object_id, order_list>& get_sell_order_list() const { return m_sell_orders; };

private:
  std::queue<order> m_order_queue;
  std::map<object_id, order_list> m_buy_orders;
  std::map<object_id, order_list> m_sell_orders;
  unsigned m_execution_rate;
};
