#pragma once

#include "include/object.hpp"

#include <SFML/Graphics.hpp>


template<class world>
class object_panel : public sf::Drawable {

public:
  object_panel(const world& w, const sf::FloatRect& area, const sf::Font& font) : m_world(w), m_area(area), m_font(font) {};

private:
  virtual void draw(sf::RenderTarget& target,sf::RenderStates states) const
  {
    std::stringstream ss;
    for (auto obj : m_world.get_objects())
    {
      ss << obj.get_id() << " : " << std::setw(6) << std::setprecision(4) << obj.get_price() << " $\n";
    }
    sf::Text objects_text;
    objects_text.setFont(m_font);
    objects_text.setString(ss.str());
    objects_text.setCharacterSize(10); // in pixels, not points!
    objects_text.setFillColor(sf::Color::Red);
    objects_text.setPosition(get_position());
    target.draw(objects_text);
  }

  inline sf::Vector2f get_position() const
  {
    return sf::Vector2f(m_area.left, m_area.top);
  }

  const world& m_world;
  const sf::FloatRect& m_area;
  const sf::Font& m_font;
};
