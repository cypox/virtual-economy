#pragma once

#include "actor.hpp"
#include "object.hpp"


class transaction {
public:
  transaction() : m_buyer(-1), m_seller(-1), m_object(-1), m_quantity(0), m_price(0), m_empty(true) { };
  transaction(actor_id buyer, actor_id seller, object_id obj, unsigned qua, double price) : m_buyer(buyer), m_seller(seller), m_object(obj), m_quantity(qua), m_price(price), m_empty(false) { };

  actor_id get_buyer() const { return m_buyer; };
  actor_id get_seller() const { return m_seller; };
  object_id get_object() const { return m_object; };
  unsigned get_quantity() const { return m_quantity; };
  double get_price() const { return m_price; };
  bool is_empty() const { return m_empty; };

private:
  bool m_empty = true;
  actor_id m_buyer;
  actor_id m_seller;
  object_id m_object;
  unsigned m_quantity;
  double m_price;
};
