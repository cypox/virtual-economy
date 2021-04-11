#include "include/veco.hpp"


using world_t = world<basic_logic>;

int main(int argc, char** argv)
{
  world_t simulation_world;
  simulation_world.init();
  sf::RenderWindow window(sf::VideoMode(800, 600), "VECO");
  window.setFramerateLimit(60);

  render<world_t> renderer(window, simulation_world);

  sf::Thread thread(&render<world_t>::run, &renderer);
  thread.launch();

  while (window.isOpen())
  {
    simulation_world.step();
  }
  return 0;
}
