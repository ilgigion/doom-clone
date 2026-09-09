cmake_minimum_required(VERSION 3.10)
project(DoomClone)

set(CMAKE_CXX_STANDARD 23)

set(SDL2_PATH "D:/Dev/Library/SDL2-2.32.10/x86_64-w64-mingw32" CACHE PATH "SDL2 MinGW development package")
set(SDL2_MIXER_PATH "D:/Dev/Library/SDL2_mixer-2.8.2/x86_64-w64-mingw32" CACHE PATH "SDL2_mixer MinGW development package")

if(NOT EXISTS "${SDL2_PATH}/include/SDL2/SDL.h" OR
   NOT EXISTS "${SDL2_MIXER_PATH}/include/SDL2/SDL_mixer.h")
    message(FATAL_ERROR "SDL2 development packages not found. Set SDL2_PATH and SDL2_MIXER_PATH to their MinGW x64 directories.")
endif()

include_directories("${SDL2_PATH}/include")
include_directories("${SDL2_PATH}/include/SDL2")
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
        include/FrameProjection.h
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
        tests/FloorRenderingTests.cpp
        tests/SimulationTests.cpp
        tests/EffectsTests.cpp
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

set(runtime_directory "${CMAKE_CURRENT_BINARY_DIR}")
if(CMAKE_CONFIGURATION_TYPES)
    string(APPEND runtime_directory "/$<CONFIG>")
endif()
add_custom_target(DoomClone_runtime
        COMMAND ${CMAKE_COMMAND} -E make_directory "${runtime_directory}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        "${runtime_directory}/SDL2.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_MIXER_PATH}/bin/SDL2_mixer.dll"
        "${runtime_directory}/SDL2_mixer.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${runtime_directory}/assets"
        VERBATIM
)
foreach(game_target DoomClone DoomClone_tests)
    add_dependencies(${game_target} DoomClone_runtime)
endforeach()

enable_testing()
add_executable(DoomClone_projection_tests tests/ProjectionTests.cpp)
add_test(NAME projection_math COMMAND DoomClone_projection_tests)
add_test(NAME DoomClone_tests COMMAND DoomClone_tests)
add_test(NAME floor_rendering COMMAND DoomClone_tests --floor-tests)
add_test(NAME sprite_rendering COMMAND DoomClone_tests --sprite-tests)
add_test(NAME simulation COMMAND DoomClone_tests --simulation-tests)
add_test(NAME effects COMMAND DoomClone_tests --effects-tests)
add_test(NAME wall_textures COMMAND DoomClone_tests --wall-texture-tests)
set_tests_properties(effects PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" TIMEOUT 15)
set_tests_properties(simulation PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" TIMEOUT 60)
set_tests_properties(floor_rendering sprite_rendering wall_textures PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        TIMEOUT 60
        RESOURCE_LOCK render_artifacts
)
set_tests_properties(DoomClone_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        TIMEOUT 60
)
foreach(failure video renderer)
    add_test(NAME ${failure}_initialization_failure
            COMMAND DoomClone_tests --${failure}-failure)
    set_tests_properties(${failure}_initialization_failure PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            TIMEOUT 15
    )
endforeach()
set_tests_properties(video_initialization_failure PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=invalid_audit_driver")
