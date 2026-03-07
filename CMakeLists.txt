cmake_minimum_required(VERSION 3.10)
project(DoomClone)
set(CMAKE_CXX_STANDARD 17)

set(SDL2_PATH "C:/dev/SDL2/x86_64-w64-mingw32")

include_directories(${SDL2_PATH}/include)
link_directories(${SDL2_PATH}/lib)

include_directories(include)
add_executable(DoomClone src/main.cpp)
target_link_libraries(DoomClone mingw32 SDL2main SDL2)

add_custom_command(TARGET DoomClone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_PATH}/bin/SDL2.dll"
    $<TARGET_FILE_DIR:DoomClone>/SDL2.dll
)