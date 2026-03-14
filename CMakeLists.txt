cmake_minimum_required(VERSION 3.10)
project(DoomClone)

set(CMAKE_CXX_STANDARD 23)

set(SDL2_PATH "C:/dev/SDL2/x86_64-w64-mingw32")
set(SDL2_MIXER_PATH "C:/dev/SDL2_mixer/x86_64-w64-mingw32")

include_directories("${SDL2_PATH}/include")
include_directories("${SDL2_MIXER_PATH}/include")
include_directories("include")

# path to lib
link_directories("${SDL2_PATH}/lib")
link_directories("${SDL2_MIXER_PATH}/lib")

set(SOURCES
        src/main.cpp
        src/Game.cpp
        src/Renderer.cpp
        src/Map.cpp
        src/Entity.cpp
        src/Player.cpp
        src/Enemy.cpp
        src/Menu.cpp
        src/Projectile.cpp
        src/Weapon.cpp
)

set(HEADERS
        include/Enemy.h
        include/Entity.h
        include/Player.h
        include/Menu.h
        include/Map.h
        include/Renderer.h
        include/Projectile.h
        include/Weapon.h
        include/Game.h
        include/SDLWrappers.h
)

add_executable(DoomClone ${SOURCES} ${HEADERS})


target_link_libraries(DoomClone
        mingw32
        SDL2main
        SDL2
        SDL2_mixer
)

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
        src/Menu.cpp
)
target_link_libraries(DoomClone_tests
        mingw32
        SDL2main
        SDL2
        SDL2_mixer
)

add_custom_command(TARGET DoomClone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:DoomClone>/SDL2.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_MIXER_PATH}/bin/SDL2_mixer.dll"
        $<TARGET_FILE_DIR:DoomClone>/SDL2_mixer.dll
)