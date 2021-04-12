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
  sprintf(buffer+8, ".%03d", millisec);
  return buffer;
}

template<class world>
class actor_panel : public sf::Drawable {
  using actor = world::actor_t;

public:
  actor_panel(const actor& act, const world& w, const sf::FloatRect& area, const sf::Font& font) : m_actor(act), m_world(w), m_area(area), m_font(font) {};

private:
  virtual void draw(sf::RenderTarget& target,sf::RenderStates states) const
  {
    std::stringstream ss;
    ss << m_actor.get_id() << " : " << std::setprecision(8)
        << std::put_money((m_actor.get_cash() + m_actor.get_reserved_cash()), true)
        << " (" << std::setprecision(8) << std::put_money(m_actor.get_cash(), true) << " liquid)\n";

    ss << "has: " << m_actor.get_remaining_storage() << "\n";
    for (auto obj : m_world.get_objects())
    {
      object_id oid = obj.get_id();
      ss << oid << " : " << m_actor.get_stock(oid) << " units and " << m_actor.get_reserve(oid) << " in reserve\n";
    }
    sf::Text actor_text;
    actor_text.setFont(m_font);
    actor_text.setString(ss.str());
    actor_text.setCharacterSize(16); // in pixels, not points!
    actor_text.setFillColor(sf::Color::Red);
    actor_text.setPosition(get_position());
    target.draw(actor_text);
  }

  inline sf::Vector2f get_position() const
  {
    return sf::Vector2f(m_area.left, m_area.top);
  }

  const actor& m_actor;
  const world& m_world;
  const sf::FloatRect& m_area;
  const sf::Font& m_font;
};

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
    sf::RectangleShape shape(sf::Vector2f(800, 600));
    shape.setFillColor(sf::Color::White);
    m_window.draw(shape);

    render_information(sf::FloatRect(600, 500, 200, 100));

    render_object_prices(sf::Vector2f(600, 20));

    render_transactions(sf::Vector2f(20, 500));

    render_actors(sf::FloatRect(0, 0, 600, 200));

    render_orders(sf::Vector2f(20, 200));
  }

  grid split_area(const sf::FloatRect& total_area, int rows, int cols)
  {
    grid g;
    float unit_height = total_area.height / (rows * cols);
    float unit_width = total_area.width / (rows * cols);
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
    }
    return g;
  }

  sf::Text prepare_text(const std::string& str, sf::Vector2f position, unsigned int size = 16)
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
    std::stringstream ss;
    ss << std::setw(64) << std::internal << "iteration: " << m_world.get_time() << "\n"
       << std::setw(64) << std::right << "p ==> stepping mode" << "\n"
       << std::setw(64) << std::right << "s ==> start/stop simulation" << "\n"
       << std::setw(64) << std::right << "t ==> disable stepping" << "\n"
       << std::setw(64) << std::right << "q ==> exit" << "\n"
       << std::setw(64) << std::right << get_time();
    sf::Text info_text = prepare_text(ss.str(), sf::Vector2f(area.left, area.top), 10);
    sf::FloatRect bounds = info_text.getLocalBounds();
    info_text.setPosition(area.left + area.width - bounds.width - 10, area.top + area.height - bounds.height);
    m_window.draw(info_text);
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

  void render_actors(sf::FloatRect area)
  {
    int rows = 2, cols = 2;
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
    sf::Vector2f draw_position(area.left, area.top);
    sf::Text total_money_text = prepare_text(ss.str(), draw_position);
    m_window.draw(total_money_text);
  }

  void render_orders(sf::Vector2f position)
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

private:
  sf::RenderWindow& m_window;
  const world& m_world;
  sf::Font m_font;
};
