#pragma once

#include "actor.hpp"
#include "object.hpp"
#include "market.hpp"

#include <vector>
#include <queue>
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
  world(long seed, bool running = false, bool stepping = true) : m_running(running), m_stepping(stepping)
  {
    setup_rng(seed);
    m_block_mtx.lock();
    if (m_stepping)
    {
      m_stepping_mtx.lock();
    }
  }

  void init()
  {
    generate_random_world();
  }

  void generate_random_world()
  {
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
    if (!m_running)
    {
      m_block_mtx.lock();
    }

    if (m_stepping)
    {
      m_stepping_mtx.lock();
    }

    order_list order_list;

    for (actor<logic> act : m_actors)
    {
      act.step(order_list);
    }

    m_exchange.step(order_list, m_last_transactions);

    while(!m_last_transactions.empty())
    {
      settle(m_last_transactions.front());
      m_last_transactions.pop();
    }

    m_time++;
  }

  bool settle(const transaction& t)
  {
    if (t.is_empty())
    {
      return false;
    }

    actor_id buyer = t.get_buyer();
    actor_id seller = t.get_seller();

    bool success = true;

    success = m_actors[buyer].execute_buy(t.get_object(), t.get_quantity(), t.get_price());
    if (success)
    {
      success = m_actors[seller].execute_sell(t.get_object(), t.get_quantity(), t.get_price());
    }

    if (success)
    {
      m_objects[t.get_object()].set_price(t.get_price());
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
  }

  const std::vector<actor<logic>>& get_actors() const { return m_actors; }

  const std::vector<object>& get_objects() const { return m_objects; }

  double get_price(object_id oid) const { return m_objects[oid].get_price(); }

  double get_producers_consumers_rate(object_id oid) const { return m_produce_consume_rates[oid]; }

  unsigned get_time() const { return m_time; }

  bool is_running() const { return m_running; }

  bool is_stepping() const { return m_stepping; }

  void stop()
  {
    m_running_mtx.lock();
    m_running = false;
    m_running_mtx.unlock();
  }

  void start()
  {
    m_running_mtx.lock();
    m_running = true;
    m_running_mtx.unlock();
    m_block_mtx.unlock();
  }

  void next_step()
  {
    if (m_running)
    {
      if (!m_stepping)
      {
        m_stepping = true;
      }
      m_stepping_mtx.unlock();
    }
  }

  void disable_stepping()
  {
    m_stepping = false;
    m_stepping_mtx.unlock();
  }

  void unlock_mtx()
  {
    if (!m_running)
    {
      m_block_mtx.unlock();
    }
    m_stepping_mtx.unlock();
  }

  const market& get_market() const { return m_exchange; };

private:
  std::vector<actor<logic>> m_actors;
  std::vector<object> m_objects;
  std::vector<double> m_produce_consume_rates;
  market m_exchange;
  std::queue<transaction> m_last_transactions;

  bool m_running;
  bool m_stepping;
  unsigned m_time = 0;
  std::mutex m_running_mtx;
  std::mutex m_block_mtx;
  std::mutex m_stepping_mtx;
};
