#pragma once

class pop {
public:
  pop();

  void feed();

  void work();

  void trade();

  unsigned get_size() const;

  void set_size(unsigned);

private:
  unsigned m_size;
};
