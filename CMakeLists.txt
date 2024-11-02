cmake_minimum_required(VERSION 3.30)

project(some_game_with_timer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 14)

set(SOURCE)
file(GLOB SOURCE "src/*.cpp")

if(WIN32)
add_executable(some_game_with_timer ${SOURCE})
elseif(UNIX)
add_executable(some_game_with_timer ${SOURCE})
else()
message("Unknown platform")
endif()

include_directories(some_game_with_timer "./includes")

find_package(Vulkan)
if(Vulkan_FOUND)
target_link_libraries(some_game_with_timer PRIVATE Vulkan::Vulkan)
target_include_directories(some_game_with_timer PRIVATE Vulkan::Vulkan)
else()
target_link_directories(some_game_with_timer PRIVATE "./vulkan_not_found_libs")
target_link_libraries(some_game_with_timer PRIVATE "vulkan-1")
target_include_directories(some_game_with_timer PRIVATE "./vulkan_not_found_includes")
endif()
