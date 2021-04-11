#pragma once

#include <cmath>
#include <sys/time.h>
#include <iomanip>
#include <sstream>
#include <SFML/Graphics.hpp>


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

template<class world>
class render {
public:
  render() = delete;

  render(sf::RenderWindow& window, world& w) : m_window(window), m_world(w)
  {
    if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
      throw std::runtime_error("font not found!");
    }
  }

  sf::Text prepare_text(const std::string& str, sf::Vector2f& position)
  {
    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(16); // in pixels, not points!
    text.setFillColor(sf::Color::Red);
    text.setPosition(position);
    return text;
  }

  void render_world(sf::RenderWindow& window, world& w)
  {
    sf::RectangleShape shape(sf::Vector2f(800.f, 600.f));
    shape.setFillColor(sf::Color::White);
    window.draw(shape);

    sf::Vector2f position(20.f, 20.f);
    std::stringstream ss;
    ss << get_time();
    window.draw(prepare_text(ss.str(), position));
    position.y += 16;

    for (auto obj : w.get_objects())
    {
      std::stringstream ss;
      ss << obj.get_id() << " : " << std::setw(6) << std::setprecision(4) << obj.get_price() << " $";
      window.draw(prepare_text(ss.str(), position));
      position.y += 16;
    }
  }

  void run()
  {
    m_window.setActive(true);

    while (m_window.isOpen())
    {
      sf::Event event;
      while (m_window.pollEvent(event))
      {
        if (event.type == sf::Event::Closed)
          m_window.close();
      }

      m_window.clear();
      render_world(m_window, m_world);
      m_window.display();
    }
  }

private:
  sf::RenderWindow& m_window;
  world& m_world;
  sf::Font m_font;
};
