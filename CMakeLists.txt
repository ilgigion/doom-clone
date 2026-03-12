cmake_minimum_required(VERSION 3.10)
project(DoomClone)

set(CMAKE_CXX_STANDARD 20)

# path for SDL2
set(SDL2_PATH "C:/dev/SDL2/x86_64-w64-mingw32")

# add path for headings
include_directories(${SDL2_PATH}/include)
# add folzer "include"
include_directories(include)

# add path for labuary
link_directories(${SDL2_PATH}/lib)

# conect src
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Renderer.cpp
    src/Map.cpp
    src/Entity.cpp
    src/Player.cpp
        include/Enemy.h
        src/Enemy.cpp
        include/Menu.h
        src/Menu.cpp
)

add_executable(DoomClone ${SOURCES})

target_link_libraries(DoomClone mingw32 SDL2main SDL2)

# dll copying 
add_custom_command(TARGET DoomClone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_PATH}/bin/SDL2.dll"
    $<TARGET_FILE_DIR:DoomClone>/SDL2.dll
)