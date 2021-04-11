#pragma once

#include "world.hpp"
#include "actor.hpp"
#include "order.hpp"
#include "object.hpp"

#include <vector>
#include <unordered_set>


class basic_logic {
public:
  basic_logic() = delete;
  basic_logic(actor<basic_logic>* act, world<basic_logic>* w) : m_actor(act), m_world(w)
  {
    for (const object& obj : m_world->get_objects())
    {
      object_id oid = obj.get_id();
      double producers_rate = m_world->get_producers_consumers_rate(oid);
      double chance = generate_random_number<double>(0, 1);
      if (chance > producers_rate)
      {
        m_is_producing.insert(oid);
      }
      else
      {
        m_is_consuming.insert(oid);
      }
    }
  };

  void take_decision(order_list& order_list)
  {
    for (const object& obj : m_world->get_objects())
    {
      object_id obj_id = obj.get_id();
      unsigned own = m_actor->get_stock(obj_id);
      double price = m_world->get_price(obj_id);

      bool is_in_need;
      if (m_is_consuming.find(obj_id) != m_is_consuming.end())
      {
        is_in_need = m_actor->destroy(obj_id, m_consumption_rate);
      }
      if (is_in_need)
      {
        price = price * (1.0 + m_need_price_increase_factor);
      }

      if (own < m_need_threshold)
      {
        if (m_is_producing.find(obj_id) == m_is_producing.end())
        {
          double target_price = price * (1.0 + m_buy_margin_factor);
          order o = m_actor->make_order(order_type::BUY, 1, target_price, obj_id);
          if (o.is_valid())
          {
            order_list.emplace(o);
          }
        }
        else
        {
          m_actor->create(obj_id, m_production_rate);
        }
      }
      else if (own >= m_produce_threshold && own < m_liquidate_threshold)
      {
        if (m_is_producing.find(obj_id) != m_is_producing.end())
        {
          m_actor->create(obj_id, m_production_rate);
        }
      }
      else if (own >= m_liquidate_threshold && own < m_waste_threshold)
      {
        double target_price = price * (1.0 + m_sell_margin_factor);
        order o = m_actor->make_order(order_type::SELL, 1, target_price, obj_id);
        if (o.is_valid())
        {
          order_list.emplace(o);
        }
      }
      else if (own >= m_waste_threshold)
      {
        m_actor->destroy(obj_id, m_waste_rate);
      }
    }
  }

private:
  actor<basic_logic>* m_actor;
  world<basic_logic>* m_world;
  double m_need_price_increase_factor = 0.2;
  double m_buy_margin_factor = 0.01;
  double m_sell_margin_factor = 0.01;
  unsigned m_need_threshold = 10;
  unsigned m_produce_threshold = 30;
  unsigned m_liquidate_threshold = 60;
  unsigned m_waste_threshold = 100;
  unsigned m_consumption_rate = 2;
  unsigned m_production_rate = 5;
  unsigned m_waste_rate = 3;
  std::unordered_set<object_id> m_is_producing;
  std::unordered_set<object_id> m_is_consuming;
};
