cmake_minimum_required(VERSION 3.23)
project(PCBCTB)

set(CMAKE_CXX_STANDARD 14)

add_executable(PCBCTB
	src/main.cpp
	src/ctb.cpp
	src/rle.cpp)

target_link_libraries(PCBCTB lunasvg)
