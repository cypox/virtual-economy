cmake_minimum_required (VERSION 2.8.11)
project (veco)

#target_include_directories (virtual-env PUBLIC include)
target_sources(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
  PUBLIC
)
