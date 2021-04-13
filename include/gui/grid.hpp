#pragma once

#include <vector>
#include <SFML/Graphics.hpp>


class grid {
  using grid_t = std::vector<std::vector<sf::FloatRect>>;

public:
  grid() = default;

  grid(const sf::FloatRect& total_area, int rows, int cols)
  {
    m_rows = rows;
    m_cols = cols;
    float unit_height = total_area.height / m_rows;
    float unit_width = total_area.width / m_cols;
    int r = 0, c = 0;
    m_grid.resize(m_rows);
    for (auto& row : m_grid)
    {
      row.resize(m_cols);
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
  }
  
  const sf::FloatRect get_area(int row, int col) const { return m_grid[row][col]; }
  inline int get_rows() const { return m_rows; }
  inline int get_cols() const { return m_cols; }

private:
  grid_t m_grid;
  int m_rows;
  int m_cols;
};
