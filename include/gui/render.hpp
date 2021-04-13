#pragma once

#include "include/gui/actor_panel.hpp"
#include "include/gui/object_panel.hpp"
#include "include/gui/order_panel.hpp"

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
  sprintf(buffer+8, ".%03d", millisec);
  return buffer;
}

template<class world>
class render {
public:
  render() = delete;

  render(sf::RenderWindow& window, const world& w) : m_window(window), m_world(w), m_object_panel(w), m_actor_panel(w), m_order_panel(w)
  {
    if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"))
    {
      throw std::runtime_error("font not found!");
    }
  }

  void run()
  {
    m_window.setActive(true);

    while (m_window.isOpen())
    {
      handle_events();
      m_window.clear();
      render_world();
      m_window.display();
    }
  }

  void handle_events()
  {
    sf::Event event;
    while (m_window.pollEvent(event))
    {
      switch (event.type)
      {
        case sf::Event::Closed:
          m_window.close();
          m_world.unlock_mtx();
          break;
        
        case sf::Event::Resized:
          event.size.width;
          event.size.height;
          break;

        case sf::Event::KeyPressed:
          switch (event.key.code)
          {
            case sf::Keyboard::S:
              if (m_world.is_running())
                m_world.stop();
              else
                m_world.start();
              break;
            case sf::Keyboard::Q:
              m_window.close();
              m_world.unlock_mtx();
            case sf::Keyboard::P:
              m_world.next_step();
              break;
            case sf::Keyboard::T:
              m_world.disable_stepping();
              break;
          }
          break;

        default:
          break;
      }
    }
  }

  void render_world()
  {
    struct timeval start, end;
    gettimeofday(&start, nullptr);

    sf::RectangleShape shape(sf::Vector2f(m_window.getSize().x, m_window.getSize().y));
    shape.setFillColor(sf::Color::White);
    m_window.draw(shape);

    render_information(sf::FloatRect(600, 500, 200, 100));

    render_object_prices(sf::FloatRect(600, 0, 200, 200));

    render_transactions(sf::FloatRect(0, 500, 600, 100));

    render_actors(sf::FloatRect(0, 0, 600, 200));

    render_orders(sf::FloatRect(0, 200, 800, 300));

    gettimeofday(&end, nullptr);
    m_render_time = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
  }

  void draw_background(sf::FloatRect area, sf::Color outline, sf::Color fill = sf::Color::Transparent)
  {
    sf::RectangleShape background(sf::Vector2f(area.width, area.height));
    background.setPosition(area.left, area.top);
    background.setOutlineThickness(2);
    background.setOutlineColor(outline);
    background.setFillColor(fill);
    m_window.draw(background);
  }

  sf::Text prepare_text(const std::string& str, sf::Vector2f position, unsigned int size = 10)
  {
    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(size); // in pixels, not points!
    text.setFillColor(sf::Color::Red);
    text.setPosition(position);
    return text;
  }

  void render_information(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::Yellow);

    std::stringstream ss;
    ss << std::setw(30) << std::right << "p ==> step. mode" << "\n"
       << std::setw(30) << std::right << "s ==> start/stop" << "\n"
       << std::setw(30) << std::right << "t ==> nostepping" << "\n"
       << std::setw(30) << std::right << "q ==> exit prog." << "\n"
       << "iteration:"
       << std::setw(20) << std::right << m_world.get_time() << "\n"
       << "last iteration time:"
       << std::setw(8) << std::right << m_world.get_iteration_time() / 1000.f << std::setw(2) << "ms\n"
       << "render time:"
       << std::setw(16) << std::right << m_render_time / 1000.f << std::setw(2) << "ms\n"
       << "time:"
       << std::setw(25) << std::right << get_time() << "\n";
    sf::Text info_text = prepare_text(ss.str(), sf::Vector2f(area.left, area.top), 8);
    sf::FloatRect bounds = info_text.getLocalBounds();
    info_text.setPosition(area.left + area.width - bounds.width - 10, area.top + area.height - bounds.height);
    m_window.draw(info_text);
  }

  void render_object_prices(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::Green);

    m_object_panel.update(&area, &m_font);
    m_window.draw(m_object_panel);
  }

  void render_actors(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::Yellow);

    m_actor_panel.update(&area, &m_font, 4, 4);
    m_window.draw(m_actor_panel);
  }

  void render_orders(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::White);

    m_order_panel.update(&area, &m_font, 1, 6);
    m_window.draw(m_order_panel);
  }

  void render_transactions(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::Blue);

    sf::Vector2f position(area.left, area.top);
    std::stringstream ss;
    std::list<transaction> tr_list = m_world.get_last_transactions();
    if (tr_list.empty())
    {
      return;
    }
    for (transaction tr : tr_list)
    {
      ss.str("");
      ss << tr.get_buyer() << " BOUGHT " << tr.get_object() << " FROM " << tr.get_seller() << " @ " << tr.get_price();
      m_window.draw(prepare_text(ss.str(), position));
      position.y += 16;
    }
  }

private:
  object_panel<world> m_object_panel;
  actor_panel<world> m_actor_panel;
  order_panel<world> m_order_panel;
  sf::RenderWindow& m_window;
  const world& m_world;
  sf::Font m_font;
  long m_render_time = 0;
};
