cmake_minimum_required(VERSION 3.30)

project(Pershit)


if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")) # i HATE MSVC
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
endif()

set(SOURCE)
file(GLOB SOURCE "src/*.cpp")

# if(CMAKE_BUILD_TYPE STREQUAL "Release")
#     set(WIN32_GUI_FLAG WIN32)
# else()
#     set(WIN32_GUI_FLAG "")
# endif()
# 
# add_executable(Pershit ${WIN32_GUI_FLAG} ${SOURCE})
if(WIN32)
    add_executable(Pershit ${SOURCE})
    target_sources(Pershit PRIVATE "resources/resource.rc")

    include_directories(Pershit "./includes")

    find_package(Vulkan)
    if(Vulkan_FOUND)
        target_link_libraries(Pershit PRIVATE Vulkan::Vulkan)
        target_include_directories(Pershit PRIVATE Vulkan::Vulkan)
    else()
        target_link_directories(Pershit PRIVATE "./vulkan_not_found_libs")
        target_link_libraries(Pershit PRIVATE "vulkan-1")
        target_include_directories(Pershit PRIVATE "./vulkan_not_found_includes")
    endif()

    if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")) # i HATE MSVC
        target_compile_options(Pershit PRIVATE "/Zc:__cplusplus")
    endif()
else()
    message("Unknown platform")
endif()

