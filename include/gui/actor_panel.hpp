#pragma once

#include "include/object.hpp"

#include <sstream>
#include <iomanip>
#include <SFML/Graphics.hpp>


template<class world>
class actor_panel : public sf::Drawable {
  using actor = world::actor_t;

public:
  actor_panel(const actor& act, const world& w, const sf::FloatRect& area, const sf::Font& font) : m_actor(act), m_world(w), m_area(area), m_font(font) {};

  inline sf::Vector2f get_position() const
  {
    return sf::Vector2f(m_area.left, m_area.top);
  }

private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    std::stringstream ss;
    ss << m_actor.get_id() << " : " << std::setprecision(8)
        << std::put_money((m_actor.get_cash() + m_actor.get_reserved_cash()), true)
        << " (" << std::setprecision(8) << std::put_money(m_actor.get_cash(), true) << " liquid)\n";

    ss << "has: " << m_actor.get_remaining_storage() << "\n";
    for (auto obj : m_world.get_objects())
    {
      object_id oid = obj.get_id();
      //ss << oid << " : " << m_actor.get_stock(oid) << " units and " << m_actor.get_reserve(oid) << " in reserve\n";
    }
    sf::Text actor_text;
    actor_text.setFont(m_font);
    actor_text.setString(ss.str());
    actor_text.setCharacterSize(10); // in pixels, not points!
    actor_text.setFillColor(sf::Color::Red);
    actor_text.setPosition(get_position());
    target.draw(actor_text);
  }

  const actor& m_actor;
  const world& m_world;
  const sf::FloatRect& m_area;
  const sf::Font& m_font;
};
