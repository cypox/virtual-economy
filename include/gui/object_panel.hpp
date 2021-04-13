#pragma once

#include "include/object.hpp"

#include <vector>
#include <deque>
#include <array>
#include <SFML/Graphics.hpp>


template<class world>
class object_panel : public sf::Drawable {

public:
  object_panel(const world& w) : m_world(w)
  {
    m_num_objects = m_world.get_objects().size();
    m_price_history.resize(m_num_objects);
  };

  void update(const sf::FloatRect* area, const sf::Font* font)
  {
    m_area = area;
    m_font = font;
    m_area_grid = grid(*m_area, m_num_objects, 1);
    if (m_last_timestep != m_world.get_time())
    {
      m_last_timestep = m_world.get_time();
      for (int i = 0 ; i < m_num_objects ; ++ i)
      {
        object o = m_world.get_objects()[i];
        if (m_price_history[i].size() == m_graph_width_resolution)
        {
          m_price_history[i].pop_front();
          m_price_history[i].emplace_back(m_last_timestep, o.get_price());
        }
        else
        {
          m_price_history[i].emplace_back(m_last_timestep, o.get_price());
        }
      }
    }
  }

  void draw_graph(const object& obj, const sf::FloatRect& area, sf::RenderTarget& target, sf::RenderStates& states) const
  {
    std::deque<std::pair<unsigned, double>> points = m_price_history[obj.get_id()];
    float x_0 = area.left, y_0 = area.top;
    float x_increment = area.width/(m_graph_width_resolution-1), y_increment = area.height/m_graph_height_resolution;
    int idx = 0;
    for (auto it = points.begin() ; it != points.end() && it + 1 != points.end() ; ++ it)
    {
      float x1 = area.left + idx * x_increment;
      float x2 = area.left + (idx + 1) * x_increment;
      float y1 = area.top + area.height - it->second * y_increment;
      float y2 = area.top + area.height - (it+1)->second * y_increment;
      sf::Vertex segment[2] = {
        {{x1, y1}, sf::Color::Black},
        {{x2, y2}, sf::Color::Black}
      };
      target.draw(segment, 2, sf::Lines, states);
      ++ idx;
    }
    std::stringstream ss;
    ss << std::setw(6) << std::setprecision(4) << obj.get_price() << " $\n";
    sf::Text objects_text;
    objects_text.setFont(*m_font);
    objects_text.setString(ss.str());
    objects_text.setCharacterSize(10);
    objects_text.setFillColor(sf::Color::Red);
    objects_text.setPosition(area.left + area.width/2 - objects_text.getLocalBounds().width/2, area.top);
    target.draw(objects_text, states);
  }

private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    int row = 0, col = 0;
    for (auto obj : m_world.get_objects())
    {
      draw_graph(obj, m_area_grid.get_area(row, col), target, states);
      col ++;
      if (col == m_area_grid.get_cols())
      {
        col = 0;
        row ++;
        if (row == m_area_grid.get_rows())
        {
          break;
        }
      }
    }
  }

  size_t m_num_objects;
  unsigned m_last_timestep = 0;
  int m_graph_width_resolution = 100;
  int m_graph_height_resolution = 50;
  std::vector<std::deque<std::pair<unsigned, double>>> m_price_history;
  const world& m_world;
  grid m_area_grid;
  const sf::FloatRect* m_area;
  const sf::Font* m_font;
};
