cmake_minimum_required(VERSION 3.10)
project(DoomClone)

set(CMAKE_CXX_STANDARD 20)

set(SDL2_PATH "C:/dev/SDL2/x86_64-w64-mingw32")

include_directories(${SDL2_PATH}/include)
include_directories(include)

link_directories(${SDL2_PATH}/lib)

set(SOURCES
        src/main.cpp
        src/Game.cpp
        src/Renderer.cpp
        src/Map.cpp
        src/Entity.cpp
        src/Player.cpp
        src/Enemy.cpp
        src/Projectile.cpp
        src/Weapon.cpp
)

set(HEADERS
        include/Enemy.h
        include/Entity.h
        include/Player.h
        include/Map.h
        include/Renderer.h
        include/Projectile.h
        include/Weapon.h
        include/Game.h
)

add_executable(DoomClone ${SOURCES} ${HEADERS})
target_link_libraries(DoomClone mingw32 SDL2main SDL2)

add_executable(DoomClone_tests
        tests/Tests.cpp
        src/Entity.cpp
        src/Player.cpp
        src/Map.cpp
        src/Enemy.cpp
        src/Projectile.cpp
        src/Weapon.cpp
        src/Renderer.cpp
        src/Game.cpp
)
target_link_libraries(DoomClone_tests mingw32 SDL2main SDL2)

add_custom_command(TARGET DoomClone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:DoomClone>/SDL2.dll
)

add_custom_command(TARGET DoomClone_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:DoomClone_tests>/SDL2.dll
)