#include "include/veco.hpp"

#include <iostream>

int main(int argc, char** argv)
{
  std::cout << "virtual-economy" << std::endl;
  world w;
  w.init(10);
  while(true)
  {
    w.step();
  }
  return 0;
}
