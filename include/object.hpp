#pragma once

class object {
public:
  object();

  void feed();

  double get_price() const;

  void set_price(double);

private:
  double m_price;
};
