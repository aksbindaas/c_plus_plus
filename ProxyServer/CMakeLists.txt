cmake_minimum_required(VERSION 3.10)

project(Proxy VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
include_directories(include)
file(GLOB SOURCES "src/*.cpp")

#add_library(proxy MODULE ${SOURCES})

add_executable (proxy src/main.cpp ${SOURCES})

