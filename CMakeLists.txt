cmake_minimum_required(VERSION 3.10)
project(DoomClone)

set(CMAKE_CXX_STANDARD 20)

# path for SDL2
set(SDL2_PATH "C:/dev/SDL2/x86_64-w64-mingw32")

# add path for headings
include_directories(${SDL2_PATH}/include)
# add folder "include"
include_directories(include)

# add path for library
link_directories(${SDL2_PATH}/lib)

# main game sources (WITHOUT tests)
set(SOURCES
        src/main.cpp
        src/Game.cpp
        src/Renderer.cpp
        src/Map.cpp
        src/Entity.cpp
        src/Player.cpp
        src/Enemy.cpp
)

# create main game executable
add_executable(DoomClone ${SOURCES})

target_link_libraries(DoomClone mingw32 SDL2main SDL2)

# dll copying for main game
add_custom_command(TARGET DoomClone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:DoomClone>/SDL2.dll
)

#*****TESTS*****
add_executable(DoomClone_tests
        tests/Tests.cpp
        src/Entity.cpp
        src/Player.cpp
        src/Map.cpp
        src/Enemy.cpp
)

target_include_directories(DoomClone_tests PRIVATE
        ${SDL2_PATH}/include
        include
)

# Force console subsystem (use main() not WinMain())
set_target_properties(DoomClone_tests PROPERTIES LINK_FLAGS "-mconsole")

# Link libraries
target_link_libraries(DoomClone_tests
        SDL2
        mingw32
)

add_custom_command(TARGET DoomClone_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:DoomClone_tests>/SDL2.dll
)