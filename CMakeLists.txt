cmake_minimum_required(VERSION 3.15)

project(BorderlessExample)

add_executable(BorderlessExample)

target_sources(BorderlessExample
PRIVATE
    main.cpp
)

target_include_directories(BorderlessExample PRIVATE imgui ../SDL-1/include)

set(SDL_STATIC ON)
add_subdirectory(SDL-1)

target_link_libraries(BorderlessExample 
PRIVATE 
    SDL2::SDL2-static
    SDL2::SDL2main
)
