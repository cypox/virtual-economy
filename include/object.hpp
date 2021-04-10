#pragma once


using object_id = unsigned;

class object {
public:
  object() = delete;

  object(object_id id) : m_id(id), m_price(0.0) { };

  double get_price() const
  {
    return m_price;
  }

  void set_price(double price)
  {
    m_price = price;
  }

  object_id get_id() const
  {
    return m_id;
  }

private:
  object_id m_id;
  double m_price;
};
