#pragma once

#include "include/order.hpp"

#include <queue>


class market {
public:
  market()
  {
  }

  void place_order(const order o)
  {
    m_placed_orders.push(o);
  }

  void execute_single_order()
  {
    order o = m_placed_orders.front();
    m_placed_orders.pop();
  }

private:
  std::queue<order> m_placed_orders;
};
