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
    m_price_history.resize(m_world.get_objects().size());
  };

  void update(const sf::FloatRect* area, const sf::Font* font)
  {
    m_area = area;
    m_font = font;
    if (m_last_timestep != m_world.get_time())
    {
      m_last_timestep = m_world.get_time();
      size_t num_objects = m_world.get_objects().size();
      for (int i = 0 ; i < num_objects ; ++ i)
      {
        object o = m_world.get_objects()[i];
        if (m_price_history[i].size() == m_graph_width)
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

  void draw_graph(sf::RenderTarget& target, sf::RenderStates& states, const sf::FloatRect& area) const
  {
    std::deque<std::pair<unsigned, double>> points = m_price_history[0];
    float x_0 = area.left, y_0 = area.top;
    float x_increment = area.width/m_graph_width, y_increment = area.height/m_graph_height;
    int idx = 0;
    for (auto it = points.begin() ; it != points.end() && it + 1 != points.end() ; ++ it)
    {
      float x1 = area.left + idx * x_increment;
      float x2 = area.left + (idx + 1) * x_increment;
      float y1 = area.top + area.height - it->second * y_increment;
      float y2 = area.top + area.height - (it+1)->second * y_increment;
      //printf("(%f, %f) -> (%f, %f)\n", x1, y1, x2, y2);
      sf::Vertex segment[2] = {
        {{x1, y1}, sf::Color::Black},
        {{x2, y2}, sf::Color::Black}
      };
      target.draw(segment, 2, sf::Lines, states);
      ++ idx;
    }
  }

  inline sf::Vector2f get_position() const
  {
    return sf::Vector2f(m_area->left, m_area->top);
  }

private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    sf::FloatRect graph_area = *m_area;
    graph_area.height /= 2;
    graph_area.top = graph_area.top + graph_area.height;
    draw_graph(target, states, graph_area);

    std::stringstream ss;
    for (auto obj : m_world.get_objects())
    {
      ss << obj.get_id() << " : " << std::setw(6) << std::setprecision(4) << obj.get_price() << " $\n";
    }
    sf::Text objects_text;
    objects_text.setFont(*m_font);
    objects_text.setString(ss.str());
    objects_text.setCharacterSize(10); // in pixels, not points!
    objects_text.setFillColor(sf::Color::Red);
    objects_text.setPosition(get_position());
    target.draw(objects_text, states);
  }

  unsigned m_last_timestep = 0;
  int m_graph_width = 100;
  int m_graph_height = 50;
  std::vector<std::deque<std::pair<unsigned, double>>> m_price_history;
  const world& m_world;
  const sf::FloatRect* m_area;
  const sf::Font* m_font;
};
