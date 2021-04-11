#include "../include/veco.hpp"

#include <sys/time.h>
#include <iomanip>
#include <sstream>
#include <SFML/Graphics.hpp>


sf::Text prepare_text(const std::string& str, sf::Font& font, sf::Vector2f& position)
{
  sf::Text text;
  text.setFont(font);
  text.setString(str);
  text.setCharacterSize(16); // in pixels, not points!
  text.setFillColor(sf::Color::Red);
  text.setPosition(position);
  return text;
}

char* get_time()
{
  int millisec;
  struct timeval tv;
  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0);
  if (millisec>=1000)
  {
    millisec -=1000;
    tv.tv_sec++;
  }
  std::time_t t = std::time(nullptr);
  static char buffer[100];
  std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&t));
  sprintf(buffer+8, ".%03d\n", millisec);
  return buffer;
}

void render_world(sf::RenderWindow& window, world<basic_logic>& w)
{
  sf::RectangleShape shape(sf::Vector2f(800.f, 600.f));
  shape.setFillColor(sf::Color::White);
  window.draw(shape);

  sf::Font font;
  if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
  {
    throw std::runtime_error("font not found!");
  }

  sf::Vector2f position(20.f, 20.f);
  std::stringstream ss;
  ss << "ITERATION: " << get_time();
  window.draw(prepare_text(ss.str(), font, position));
  position.y += 16;

  for (auto obj : w.get_objects())
  {
    std::stringstream ss;
    ss << obj.get_id() << " : " << std::setprecision(4) << obj.get_price() << " $";
    window.draw(prepare_text(ss.str(), font, position));
    position.y += 16;
  }
}

int main(int argc, char** argv)
{
  world<basic_logic> simulation_world;
  simulation_world.init();
  sf::RenderWindow window(sf::VideoMode(800, 600), "VECO");
  window.setFramerateLimit(15);

  while (window.isOpen())
  {
    simulation_world.step();

    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    render_world(window, simulation_world);
    window.display();
  }
  return 0;
}
