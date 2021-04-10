#pragma once

#include "world.hpp"
#include "actor.hpp"
#include "order.hpp"
#include "object.hpp"

#include <vector>


class basic_logic {
public:
  basic_logic() = delete;
  basic_logic(actor<basic_logic>* act, world<basic_logic>* w) : m_actor(act), m_world(w) { };

  void take_decision(order_list& order_list)
  {
    for (const object& obj : m_world->get_objects())
    {
      object_id obj_id;
      m_actor->destroy(obj_id, m_consumption_rate);
      unsigned own = m_actor->get_stock(obj_id);
      double price = m_world->get_price(obj_id);
      if (own < m_buy_threshold)
      {
        double target_price = price * (1.0 + m_buy_margin_factor);
        order o = m_actor->prepare_order(order_type::BUY, 1, target_price, obj_id);
        if (o.is_valid())
        {
          order_list.emplace(o);
        }
      }
      else if (own >= m_produce_threshold && own < m_liquidate_threshold)
      {
        // TODO: buy other materials to produce other recipes
      }
      else if (own >= m_liquidate_threshold && own < m_waste_threshold)
      {
        double target_price = price * (1.0 + m_sell_margin_factor);
        order o = m_actor->prepare_order(order_type::SELL, 1, target_price, obj_id);
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
  double m_buy_margin_factor = 0.01;
  double m_sell_margin_factor = 0.01;
  unsigned m_consumption_rate = 2;
  unsigned m_waste_rate = 2;
  unsigned m_buy_threshold = 10;
  unsigned m_produce_threshold = 30;
  unsigned m_liquidate_threshold = 60;
  unsigned m_waste_threshold = 100;
};
