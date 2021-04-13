#pragma once

#include "actor_panel.hpp"

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
  using grid = std::vector<std::vector<sf::FloatRect>>;

  render() = delete;

  render(sf::RenderWindow& window, const world& w) : m_window(window), m_world(w)
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
    sf::RectangleShape shape(sf::Vector2f(m_window.getSize().x, m_window.getSize().y));
    shape.setFillColor(sf::Color::White);
    m_window.draw(shape);

    render_information(sf::FloatRect(600, 500, 200, 100));

    render_object_prices(sf::FloatRect(600, 0, 200, 200));

    render_transactions(sf::FloatRect(0, 500, 600, 100));

    render_actors(sf::FloatRect(0, 0, 600, 200));

    render_orders(sf::FloatRect(0, 200, 800, 300));
  }

  grid split_area(const sf::FloatRect& total_area, int rows, int cols)
  {
    grid g;
    float unit_height = total_area.height / rows;
    float unit_width = total_area.width / cols;
    int r = 0, c = 0;
    g.resize(rows);
    for (auto& row : g)
    {
      row.resize(cols);
      for (auto& col : row)
      {
        col.height = unit_height;
        col.width = unit_width;
        col.left = total_area.left + c * unit_width;
        col.top = total_area.top + r * unit_height;
        c ++;
      }
      r ++;
      c = 0;
    }
    return g;
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
       << std::setw(10) << std::right << m_world.get_iteration_time() / 1000 << "\n"
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

    sf::Vector2f position(area.left, area.top);
    for (auto obj : m_world.get_objects())
    {
      std::stringstream ss;
      ss << obj.get_id() << " : " << std::setw(6) << std::setprecision(4) << obj.get_price() << " $";
      m_window.draw(prepare_text(ss.str(), position));
      position.y += 16;
    }
  }

  void render_actors(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::Yellow);

    int rows = 4, cols = 4;
    grid area_grid = split_area(area, rows, cols);
    int row = 0, col = 0;
    double total_money = 0.f;
    for (auto actor : m_world.get_actors())
    {
      total_money += actor.get_cash() + actor.get_reserved_cash();
      actor_panel<world> actors_drawing(actor, m_world, area_grid[row][col], m_font);
      m_window.draw(actors_drawing);
      col ++;
      if (col == cols)
      {
        col = 0;
        row ++;
        if (row == rows)
        {
          break;
        }
      }
    }
    std::stringstream ss;
    ss << "total money is: " << total_money << "\n";
    sf::Vector2f draw_position(area.left + (area.width / 2), area.top + area.height - 10);
    sf::Text total_money_text = prepare_text(ss.str(), draw_position);
    m_window.draw(total_money_text);
  }

  void render_orders(sf::FloatRect area)
  {
    draw_background(area, sf::Color::Black, sf::Color::White);

    sf::Vector2f position(area.left, area.top);
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

      position.y = initial_y;
      position.x += 128;

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
  sf::RenderWindow& m_window;
  const world& m_world;
  sf::Font m_font;
};
