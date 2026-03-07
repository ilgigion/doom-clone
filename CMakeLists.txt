cmake_minimum_required(VERSION 3.10)
project(DoomClone)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} include)
add_executable(DoomClone src/main.cpp)

target_link_libraries(DoomClone ${SDL2_LIBRARIES})
