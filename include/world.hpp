#pragma once

#include "market.hpp"

#include <vector>
#include <random>
#include <limits>


class world {
public:
  void init(unsigned pop_size)
  {
    std::srand(time(nullptr));
    
    m_objects.reserve(3);
    for (int i = 0 ; i < 3 ; ++ i)
    {
      double x = rand();
      double price = (x / RAND_MAX) * 10.0;
      m_objects.emplace_back(i, price);
    }

    m_actors.reserve(pop_size);
    for (int i = 0 ; i < 10 ; ++ i)
    {
      double x = rand();
      double cash = ((double)x / RAND_MAX) * std::numeric_limits<double>::max();
      m_actors.emplace_back(i, cash);
      for (object obj : m_objects)
      {
        unsigned quantity = rand() % 100;
        m_actors[i].acquire(obj.get_id(), quantity);
      }
    }
  }

  void step()
  {
    order_list order_list;
    transaction result_transaction;

    for (actor act : m_actors)
    {
      act.step(order_list);
    }

    m_exchange.step(order_list, result_transaction);

    execute(result_transaction);
  }

  void execute(const transaction& tr)
  {
    if (tr.is_empty())
    {
      return;
    }

    actor_id buyer = tr.get_buyer();
    actor_id seller = tr.get_seller();

    m_actors[buyer].execute_buy(tr.get_object(), tr.get_quantity(), tr.get_price());
    m_actors[seller].execute_sell(tr.get_object(), tr.get_quantity(), tr.get_price());
  }

private:
  std::vector<actor> m_actors;
  std::vector<object> m_objects;
  market m_exchange;
};
