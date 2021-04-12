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

  void run()
  {
    m_window.setActive(true);

    while (m_window.isOpen())
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

      m_window.clear();
      render_world();
      m_window.display();
    }
  }

  render(sf::RenderWindow& window, const world& w) : m_window(window), m_world(w)
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

  void render_information(sf::Vector2f position)
  {
    std::stringstream ss;
    ss << "press:\n"
       << "p ==> stepping mode\n"
       << "s ==> start/stop simulation\n"
       << "t ==> resume simulation after stepping mode\n"
       << "q ==> exit";
    sf::Text info_text = prepare_text(ss.str(), position);
    m_window.draw(info_text);
  }

  void render_time(sf::Vector2f position)
  {
    std::stringstream ss;
    ss << get_time();
    m_window.draw(prepare_text(ss.str(), position));
  }

  void render_world_time(sf::Vector2f position)
  {
    std::stringstream ss;
    ss << "ITERATION: " << m_world.get_time();
    m_window.draw(prepare_text(ss.str(), position));
  }

  void render_object_prices(sf::Vector2f position)
  {
    for (auto obj : m_world.get_objects())
    {
      std::stringstream ss;
      ss << obj.get_id() << " : " << std::setw(6) << std::setprecision(4) << obj.get_price() << " $";
      m_window.draw(prepare_text(ss.str(), position));
      position.y += 16;
    }
  }

  void render_actors(sf::Vector2f position)
  {
    double total_money = 0.f;
    for (auto actor : m_world.get_actors())
    {
      total_money += actor.get_cash() + actor.get_reserved_cash();
      std::stringstream ss;
      ss << actor.get_id() << " : " << std::setprecision(8)
         << (unsigned)(actor.get_cash() + actor.get_reserved_cash()) << "$"
         << " (" << std::setprecision(8) << (unsigned)actor.get_cash() << "$ liquid)";
      m_window.draw(prepare_text(ss.str(), position));
      position.y += 16;

      ss.str("");
      ss << " has: " << actor.get_remaining_storage();
      m_window.draw(prepare_text(ss.str(), position));
      position.y += 16;
      position.x += 16;
      for (auto obj : m_world.get_objects())
      {
        object_id oid = obj.get_id();
        std::stringstream ss;
        ss << oid << " : " << actor.get_stock(oid) << " units and " << actor.get_reserve(oid) << " in reserve";
        m_window.draw(prepare_text(ss.str(), position));
        position.y += 16;
      }
      position.x -= 16;
    }
    position.y += 16;
    std::stringstream ss;
    ss << "total money is: " << total_money;
    m_window.draw(prepare_text(ss.str(), position));
  }

  void render_market(sf::Vector2f position)
  {
    std::stringstream ss;
    float initial_y = position.y;
    for (auto o : m_world.get_objects())
    {
      object_id oid = o.get_id();
      auto order_map = m_world.get_market().get_buy_order_list();
      int buy_size = 0;
      if (order_map.count(oid) > 0)
      {
        buy_size = order_map.at(oid).size();
        ss.str("");
        ss << "#BUY" << oid << " has " << buy_size;
        m_window.draw(prepare_text(ss.str(), position));
        position.y += 16;
        for (auto o : order_map.at(oid))
        {
          ss.str("");
          ss << o.get_actor_id() << "@" << o.get_strike();
          m_window.draw(prepare_text(ss.str(), position));
          position.y += 16;
        }
      }

      position.y += 16;

      order_map = m_world.get_market().get_sell_order_list();
      int sell_size = 0;
      if (order_map.count(oid) > 0)
      {
        sell_size = order_map.at(oid).size();
        ss.str("");
        ss << "#SELL" << oid << " has " << sell_size;
        m_window.draw(prepare_text(ss.str(), position));
        position.y += 16;
        for (auto o : order_map.at(oid))
        {
          ss.str("");
          ss << o.get_actor_id() << "@" << o.get_strike();
          m_window.draw(prepare_text(ss.str(), position));
          position.y += 16;
        }
      }

      position.x += 128;
      position.y = initial_y;
    }
  }

  void render_transactions(sf::Vector2f position)
  {
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

  void render_world()
  {
    sf::RectangleShape shape(sf::Vector2f(800, 600));
    shape.setFillColor(sf::Color::White);
    m_window.draw(shape);

    sf::RectangleShape background(sf::Vector2f(400, 100));
    background.setFillColor(sf::Color::Black);
    background.setPosition(sf::Vector2f(10, 500));
    m_window.draw(background);
    render_information(sf::Vector2f(20, 500));

    render_world_time(sf::Vector2f(20, 20));

    render_object_prices(sf::Vector2f(20, 60));

    render_transactions(sf::Vector2f(20, 320));

    render_actors(sf::Vector2f(400, 20));

    render_market(sf::Vector2f(20, 160));

    render_time(sf::Vector2f(690, 580));
  }

private:
  sf::RenderWindow& m_window;
  const world& m_world;
  sf::Font m_font;
};
