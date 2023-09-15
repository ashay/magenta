cmake_minimum_required(VERSION 3.20)

project(magenta
  LANGUAGES C CXX
  VERSION 0.0.1
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(app)
add_subdirectory(external)
add_subdirectory(lib)
add_subdirectory(tests)
