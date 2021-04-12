#include "include/veco.hpp"


using world_t = world<basic_logic>;

int main(int argc, char** argv)
{
  argument_parser parser(argc, argv);
  bool starts_running = parser.exists("--start-running");
  bool no_stepping = parser.exists("--no-stepping");
  int framerate = parser.get_int("--frame-rate", 60);
  int rng_seed = parser.get_long("--seed", time(nullptr));

  world_t simulation_world(rng_seed, starts_running || !no_stepping, !no_stepping);
  simulation_world.init();
  sf::RenderWindow window(sf::VideoMode(800, 600), "VECO");
  window.setFramerateLimit(framerate);

  render<world_t> renderer(window, simulation_world);

  sf::Thread thread(&render<world_t>::run, &renderer);
  thread.launch();

  while (window.isOpen())
  {
    simulation_world.step();
  }
  return 0;
}
