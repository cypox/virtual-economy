#pragma once

#include "include/order.hpp"
#include "include/gui/grid.hpp"

#include <iostream>
#include <sstream>
#include <SFML/Graphics.hpp>


template<class world>
class order_panel : public sf::Drawable {
public:
  order_panel(const world& w) : m_world(w)
  {
  }

  void update(const sf::FloatRect* area, const sf::Font* font, int rows, int cols)
  {
    m_area = area;
    m_font = font;
    m_area_grid = grid(*m_area, rows, cols);
  }

  void draw_buys(const object& o, const sf::FloatRect& area, sf::RenderTarget& target, sf::RenderStates states) const
  {
    sf::Text m_order_text;

    std::stringstream ss("");
    object_id oid = o.get_id();
    auto order_map = m_world.get_market().get_buy_order_list();
    int buy_size = 0;
    ss << "#BUY" << oid;
    if (order_map.count(oid) > 0)
    {
      buy_size = order_map.at(oid).size();
      ss << " has " << buy_size << "\n";
      for (auto o : order_map.at(oid))
      {
        ss << o.get_actor_id() << "@" << o.get_strike() << "\n";
      }
    }
    m_order_text.setFont(*m_font);
    m_order_text.setString(ss.str());
    m_order_text.setCharacterSize(10);
    m_order_text.setFillColor(sf::Color::Red);
    m_order_text.setPosition(area.left, area.top);

    sf::FloatRect bounds = m_order_text.getLocalBounds();
    sf::Vector2f bottom_right(area.left + bounds.width, area.top + bounds.height);
    if (area.contains(bottom_right))
    {
      target.draw(m_order_text, states);
    }
    else
    {
      sf::RenderTexture rt;
      rt.create(area.width, area.height);

      m_order_text.setPosition(0, 0);
      rt.draw(m_order_text, states);
      rt.display();

      sf::Sprite sprite;
      sprite.setTexture(rt.getTexture());
      sprite.setPosition(area.left, area.top);

      target.draw(sprite, states);
    }
  }

  void draw_sells(const object& o, const sf::FloatRect& area, sf::RenderTarget& target, sf::RenderStates states) const
  {
    sf::Text m_order_text;
    std::stringstream ss("");
    object_id oid = o.get_id();
    auto order_map = m_world.get_market().get_sell_order_list();
    int sell_size = 0;
    ss << "#SELL" << oid;
    if (order_map.count(oid) > 0)
    {
      sell_size = order_map.at(oid).size();
      ss << " has " << sell_size << "\n";
      for (auto o : order_map.at(oid))
      {
        ss << o.get_actor_id() << "@" << o.get_strike() << "\n";
      }
    }
    m_order_text.setFont(*m_font);
    m_order_text.setString(ss.str());
    m_order_text.setCharacterSize(10);
    m_order_text.setFillColor(sf::Color::Red);
    m_order_text.setPosition(area.left, area.top);

    sf::FloatRect bounds = m_order_text.getLocalBounds();
    sf::Vector2f bottom_right(area.left + bounds.width, area.top + bounds.height);
    if (area.contains(bottom_right))
    {
      target.draw(m_order_text, states);
    }
    else
    {
      sf::RenderTexture rt;
      rt.create(area.width, area.height);

      m_order_text.setPosition(0, 0);
      rt.draw(m_order_text, states);
      rt.display();

      sf::Sprite sprite;
      sprite.setTexture(rt.getTexture());
      sprite.setPosition(area.left, area.top);

      target.draw(sprite, states);
    }
  }

  void draw_single_order(const object& o, const sf::FloatRect& area, sf::RenderTarget& target, sf::RenderStates states) const
  {
    sf::FloatRect left = area, right = area;
    left.width = right.width = area.width/2;
    right.left = area.left + left.width;
    draw_buys(o, left, target, states);
    draw_sells(o, right, target, states);
  }

private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    int row = 0, col = 0;
    for (auto o : m_world.get_objects())
    {
      draw_single_order(o, m_area_grid.get_area(row, col), target, states);
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
