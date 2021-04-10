#pragma once

#include "include/object.hpp"


enum order_type {
  BUY = 1,
  SELL = 2
};

class order {
public:
  order()
  {
  }

private:
  order_type m_type;
  double m_strike;
  object_id m_object_id;
};
