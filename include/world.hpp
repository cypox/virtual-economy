#pragma once

#include "actor.hpp"
#include "object.hpp"
#include "market.hpp"

#include <vector>
#include <random>
#include <limits>
#include <mutex>


void setup_rng(long seed = time(nullptr))
{
  printf("running with seed: %ld\n", seed);
  srand(seed);
}

template<class T>
T generate_random_number(T min, T max)
{
  return min + (T)(max * (double)rand() / RAND_MAX);
}

template<class logic>
class world {
public:
  world()
  {
    m_stepping = true;
    m_block_mtx.lock();
  }

  void init()
  {
    generate_random_world();
  }

  void generate_random_world()
  {
    setup_rng(1618169463);

    m_objects.reserve(3);
    m_produce_consume_rates.reserve(3);
    for (int i = 0 ; i < 3 ; ++ i)
    {
      double price = generate_random_number<double>(0, 10);
      m_objects.emplace_back(i, price);
      double prducers_consumers_rate = generate_random_number<double>(0, 1);
      m_produce_consume_rates.emplace_back(prducers_consumers_rate);
    }

    m_actors.reserve(4);
    for (int i = 0 ; i < 4 ; ++ i)
    {
      double x = rand();
      double cash = 1000.0;
      m_actors.emplace_back(i, cash, this);
      for (object obj : m_objects)
      {
        unsigned quantity = generate_random_number<unsigned>(0, 50);
        m_actors[i].create(obj.get_id(), quantity);
      }
    }
  }

  void step()
  {
    if (!m_stepping)
    {
      m_block_mtx.lock();
    }

    order_list order_list;

    for (actor<logic> act : m_actors)
    {
      act.step(order_list);
    }

    m_exchange.step(order_list, m_last_transaction);

    settle();

    m_time++;
  }

  bool settle()
  {
    if (m_last_transaction.is_empty())
    {
      return false;
    }

    actor_id buyer = m_last_transaction.get_buyer();
    actor_id seller = m_last_transaction.get_seller();

    bool success = true;

    success = m_actors[buyer].execute_buy(m_last_transaction.get_object(), m_last_transaction.get_quantity(), m_last_transaction.get_price());
    if (success)
    {
      success = m_actors[seller].execute_sell(m_last_transaction.get_object(), m_last_transaction.get_quantity(), m_last_transaction.get_price());
    }

    if (success)
    {
      m_objects[m_last_transaction.get_object()].set_price(m_last_transaction.get_price());
      m_last_transaction.clear();
    }
    else
    {
      throw std::runtime_error("cannot settle");
    }

    return success;
  }

  void render()
  {
    printf("Population:\n");
    for (const actor<logic>& act : m_actors)
    {
      act.render();
    }
    printf("Objects:\n");
    for (const object& obj : m_objects)
    {
      obj.render();
    }
    printf("Exchange:\n");
    m_exchange.render();
    printf("Transactions:\n");
    m_last_transaction.render();
  }

  const std::vector<actor<logic>>& get_actors() const { return m_actors; }

  const std::vector<object>& get_objects() const { return m_objects; }

  double get_price(object_id oid) const { return m_objects[oid].get_price(); }

  double get_producers_consumers_rate(object_id oid) const { return m_produce_consume_rates[oid]; }

  unsigned get_time() const { return m_time; }

  bool is_running() const { return m_stepping; }

  void stop()
  {
    m_stepping_mtx.lock();
    m_stepping = false;
    m_stepping_mtx.unlock();
  }

  void start()
  {
    m_stepping_mtx.lock();
    m_stepping = true;
    m_stepping_mtx.unlock();
    m_block_mtx.unlock();
  }

private:
  std::vector<actor<logic>> m_actors;
  std::vector<object> m_objects;
  std::vector<double> m_produce_consume_rates;
  market m_exchange;
  transaction m_last_transaction;

  bool m_stepping = true;
  unsigned m_time = 0;
  std::mutex m_stepping_mtx;
  std::mutex m_block_mtx;
};
