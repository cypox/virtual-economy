#pragma once

class pop {
public:
  pop()
  {
    m_size = 0;
  }

  void feed()
  {
    printf("feeding\n");
  }

  void work()
  {
    printf("working\n");
  }

  void trade()
  {
    printf("trading\n");
  }

  unsigned get_size() const
  {
    return m_size;
  }

  void set_size(unsigned size)
  {
    m_size = size;
  }

private:
  unsigned m_size;
};
