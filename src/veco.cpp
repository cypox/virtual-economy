#include "include/veco.hpp"

#include <iostream>

int main(int argc, char** argv)
{
  std::cout << "virtual-economy" << std::endl;
  pop initial_population;
  initial_population.feed();
  initial_population.work();
  initial_population.trade();
  return 0;
}
