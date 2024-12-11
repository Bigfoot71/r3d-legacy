cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

project(r3d 
    VERSION 1.0.0
    DESCRIPTION "R3D: 3D renderer for raylib"
    LANGUAGES CXX C
)

set(R3D_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(R3D_IS_MAIN ON)
else()
    set(R3D_IS_MAIN OFF)
endif()

option(R3D_BUILD_EXAMPLES "Build the examples for the project" ${R3D_IS_MAIN})

add_subdirectory(${R3D_ROOT_PATH}/external/raylib)

include(${R3D_ROOT_PATH}/samples/CMakeLists.txt)
include(${R3D_ROOT_PATH}/src/CMakeLists.txt)

add_library(${PROJECT_NAME}
    ${R3D_SOURCES} ${R3D_SOURCES_GENERATED}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${R3D_ROOT_PATH}/external/glad
    ${R3D_ROOT_PATH}/include
)

target_link_libraries(${PROJECT_NAME} raylib)

if(R3D_BUILD_EXAMPLES)
    include(${R3D_ROOT_PATH}/examples/CMakeLists.txt)
endif()
