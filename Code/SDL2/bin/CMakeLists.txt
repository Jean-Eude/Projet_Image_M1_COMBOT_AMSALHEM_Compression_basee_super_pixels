cmake_minimum_required(VERSION 3.0)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Inclure OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


include_directories(../include ../include/SDL2 ../include/ImGui)
link_directories(../lib)

file(GLOB SOURCES ../src/*.cpp ../src/ImGui/*.cpp)

add_executable(main ${SOURCES})
target_link_libraries(main SDL2 -Ofast -fopenmp -static-libstdc++ ${OpenCV_LIBS})