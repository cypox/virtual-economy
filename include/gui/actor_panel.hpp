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
      //ss << oid << " : " << act.get_stock(oid) << " units and " << act.get_reserve(oid) << " in reserve\n";
    }
    sf::Text m_actor_text;
    m_actor_text.setFont(*m_font);
    m_actor_text.setCharacterSize(10);
    m_actor_text.setFillColor(sf::Color::Red);
    m_actor_text.setString(ss.str());
    m_actor_text.setPosition(area.left, area.top);
    
    sf::FloatRect bounds = m_actor_text.getLocalBounds();
    sf::Vector2f bottom_right(area.left + bounds.width, area.top + bounds.height);

    if (area.contains(bottom_right))
    {
      target.draw(m_actor_text, states);
    }
    else
    {
      sf::RenderTexture rt;
      rt.create(area.width, area.height);

      m_actor_text.setPosition(0, 0);
      rt.draw(m_actor_text, states);
      rt.display();

      sf::Sprite sprite;
      sprite.setTexture(rt.getTexture());
      sprite.setPosition(area.left, area.top);

      target.draw(sprite, states);
    }
  }

private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    int row = 0, col = 0;
    /*
    std::vector<actor>& actors = m_world.get_actors_copy();
    std::sort(actors.begin(), actors.end(), [](const actor& first, const actor& second){
      return first.get_total_cash() > second.get_total_cash();
    });
    */
    for (const auto& actor : m_world.get_actors())
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
    sf::Text m_total_cash_text;
    m_total_cash_text.setFont(*m_font);
    m_total_cash_text.setCharacterSize(10);
    m_total_cash_text.setFillColor(sf::Color::Red);
    std::stringstream ss("");
    ss << "total cash: " << m_world.get_total_cash();
    m_total_cash_text.setString(ss.str());
    m_total_cash_text.setPosition(m_area->left + m_area->width/2 - m_total_cash_text.getLocalBounds().width/2, m_area->top + m_area->height - m_total_cash_text.getLocalBounds().height - 10);
    target.draw(m_total_cash_text, states);
  }

  const world& m_world;
  grid m_area_grid;
  const sf::FloatRect* m_area;
  const sf::Font* m_font;
};
