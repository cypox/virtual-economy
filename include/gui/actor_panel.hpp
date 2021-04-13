#pragma once

#include "include/object.hpp"
#include "include/gui/grid.hpp"

#include <sstream>
#include <iomanip>
#include <SFML/Graphics.hpp>


template<class world>
class actor_panel : public sf::Drawable {
  using actor = world::actor_t;

public:
  actor_panel(const world& w) : m_world(w)
  {
  }

  void update(const sf::FloatRect* area, const sf::Font* font, int rows, int cols)
  {
    m_area = area;
    m_font = font;
    m_area_grid = grid(*m_area, rows, cols);
  }

  void draw_single_actor(const actor& act, const sf::FloatRect& area, sf::RenderTarget& target, sf::RenderStates states) const
  {
    std::stringstream ss;
    ss << act.get_id() << " : " << std::setprecision(8)
        << std::put_money((act.get_cash() + act.get_reserved_cash()), true)
        << " (" << std::setprecision(8) << std::put_money(act.get_cash(), true) << " liquid)\n";

    ss << "has: " << act.get_remaining_storage() << "\n";
    for (auto obj : m_world.get_objects())
    {
      object_id oid = obj.get_id();
      //ss << oid << " : " << m_actor.get_stock(oid) << " units and " << m_actor.get_reserve(oid) << " in reserve\n";
    }
    sf::Text m_actor_text;
    m_actor_text.setFont(*m_font);
    m_actor_text.setCharacterSize(10);
    m_actor_text.setFillColor(sf::Color::Red);
    m_actor_text.setString(ss.str());
    m_actor_text.setPosition(area.left, area.top);
    target.draw(m_actor_text);
  }

private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    int row = 0, col = 0;
    for (auto actor : m_world.get_actors())
    {
      draw_single_actor(actor, m_area_grid.get_area(row, col), target, states);
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

  const world& m_world;
  grid m_area_grid;
  const sf::FloatRect* m_area;
  const sf::Font* m_font;
};
