project(
  nlohmann-json
  VERSION 2.0.10
  LANGUAGES CXX
)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(
  ${PROJECT_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/json.hpp
)

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE include/
)