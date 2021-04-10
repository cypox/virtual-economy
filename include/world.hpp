#pragma once

#include "actor.hpp"
#include "object.hpp"
#include "market.hpp"

#include <vector>
#include <random>
#include <limits>


template<class logic>
class world {
public:
  void init()
  {
    generate_random_world();
  }

  void generate_random_world()
  {
    std::srand(time(nullptr));

    m_objects.reserve(3);
    for (int i = 0 ; i < 3 ; ++ i)
    {
      double x = rand();
      double price = (x / RAND_MAX) * 10.0;
      m_objects.emplace_back(i, price);
    }

    m_actors.reserve(10);
    for (int i = 0 ; i < 10 ; ++ i)
    {
      double x = rand();
      double cash = ((double)x / RAND_MAX) * 1000;
      m_actors.emplace_back(i, cash, this);
      for (object obj : m_objects)
      {
        unsigned quantity = rand() % 50;
        m_actors[i].acquire(obj.get_id(), quantity);
      }
    }
  }

  void step()
  {
    order_list order_list;
    transaction result_transaction;

    for (actor<logic> act : m_actors)
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

    m_last_prices[tr.get_object()] = tr.get_price();
  }

  const std::vector<object>& get_objects() const { return m_objects; }

  double get_price(object_id oid) const { return m_last_prices[oid]; }

private:
  std::vector<actor<logic>> m_actors;
  std::vector<object> m_objects;
  std::vector<double> m_last_prices;
  market m_exchange;
};
