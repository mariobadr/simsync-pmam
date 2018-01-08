cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(
  simsync-cl
  VERSION 0.0.1
  LANGUAGES CXX
)

add_executable(
  ${PROJECT_NAME}
  src/cxxopts.hpp
  src/main.cpp
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE simsync
)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

