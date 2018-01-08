cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(
  simsync-pmam-2018
  VERSION 0.0.1
  LANGUAGES CXX
)

add_subdirectory(external)
add_subdirectory(simsync)
add_subdirectory(simsync-cl)
