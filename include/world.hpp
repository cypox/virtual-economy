#pragma once

#include "actor.hpp"
#include "object.hpp"
#include "market.hpp"

#include <vector>
#include <queue>
#include <list>
#include <random>
#include <limits>
#include <mutex>
#include <sys/time.h>


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
  typedef actor<logic> actor_t;

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
    unsigned n_objects = 4;
    unsigned n_actors = 10;

    m_objects.reserve(n_objects);
    m_produce_consume_rates.reserve(n_objects);
    for (int i = 0 ; i < n_objects ; ++ i)
    {
      double price = generate_random_number<double>(10, 40);
      m_objects.emplace_back(i, price);
      double prducers_consumers_rate = generate_random_number<double>(0, 1);
      m_produce_consume_rates.emplace_back(prducers_consumers_rate);
    }

    m_actors.reserve(n_actors);
    for (int i = 0 ; i < n_actors ; ++ i)
    {
      double x = rand();
      double cash = 1000.0;
      m_actors.emplace_back(i, cash, this);
      for (object obj : m_objects)
      {
        unsigned quantity = generate_random_number<unsigned>(0, 0);
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

    struct timeval start, end;
    gettimeofday(&start, nullptr);

    order_list order_list;

    for (actor_t act : m_actors)
    {
      act.step(order_list);
    }

    bool end_day = ((1 + m_time) % m_trading_day_length) == 0;
    m_exchange.step(order_list, m_last_transactions, end_day);
    if (end_day)
    {
      for (actor_t& act : m_actors)
      {
        act.cancel_all_orders();
      }
    }

    m_trsc_mtx.lock();
    m_last_settled_transactions.clear();
    m_trsc_mtx.unlock();
    while(!m_last_transactions.empty())
    {
      settle(m_last_transactions.front());
      m_last_transactions.pop();
    }

    m_time++;

    gettimeofday(&end, nullptr);
    m_iter_us_mtx.lock();
    m_iter_microsecs = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    m_iter_us_mtx.unlock();
  }

  bool settle(const transaction& t)
  {
    if (t.is_empty())
    {
      return false;
    }

    bool success = m_actors[t.get_buyer()].execute_transaction(t) && m_actors[t.get_seller()].execute_transaction(t);

    if (success)
    {
      m_objects[t.get_object()].set_price(t.get_price());
      m_trsc_mtx.lock();
      m_last_settled_transactions.push_back(t);
      m_trsc_mtx.unlock();
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
    for (const actor_t& act : m_actors)
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

  double get_mkt_buy_price(object_id oid) const { return m_exchange.get_lowest_offer(oid); }

  double get_mkt_sell_price(object_id oid) const { return m_exchange.get_highest_bid(oid); }

  const std::vector<actor_t>& get_actors() const { return m_actors; }

  const std::vector<object>& get_objects() const { return m_objects; }

  double get_price(object_id oid) const { return m_objects[oid].get_price(); }

  double get_producers_consumers_rate(object_id oid) const { return m_produce_consume_rates[oid]; }

  unsigned get_time() const { return m_time; }

  bool is_running() const { return m_running; }

  bool is_stepping() const { return m_stepping; }

  void stop() const
  {
    m_running_mtx.lock();
    m_running = false;
    m_running_mtx.unlock();
  }

  void start() const
  {
    m_running_mtx.lock();
    m_running = true;
    m_running_mtx.unlock();
    m_block_mtx.unlock();
  }

  void next_step() const
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

  void disable_stepping() const
  {
    m_stepping = false;
    m_stepping_mtx.unlock();
  }

  void unlock_mtx() const
  {
    if (!m_running)
    {
      m_block_mtx.unlock();
    }
    m_stepping_mtx.unlock();
  }

  const market& get_market() const { return m_exchange; };
  std::list<transaction> get_last_transactions() const
  {
    m_trsc_mtx.lock();
    std::list<transaction> copy = m_last_settled_transactions;
    m_trsc_mtx.unlock();
    return copy;
  }

  long get_iteration_time() const
  {
    long time;
    m_iter_us_mtx.lock();
    time = m_iter_microsecs;
    m_iter_us_mtx.unlock();
    return time;
  }

  double get_total_cash() const
  {
    double sum = 0.f;
    for(const auto& a : m_actors)
    {
      sum += a.get_total_cash();
    }
    return sum;
  }

private:
  std::vector<actor_t> m_actors;
  std::vector<object> m_objects;
  std::vector<double> m_produce_consume_rates;
  market m_exchange;
  std::queue<transaction> m_last_transactions;
  std::list<transaction> m_last_settled_transactions;
  unsigned m_trading_day_length = 100;

  unsigned m_time = 0;
  long m_iter_microsecs;
  mutable std::mutex m_iter_us_mtx;

  mutable bool m_running;
  mutable bool m_stepping;
  mutable std::mutex m_running_mtx;
  mutable std::mutex m_block_mtx;
  mutable std::mutex m_stepping_mtx;
  mutable std::mutex m_trsc_mtx;
};
