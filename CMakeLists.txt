cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(raycaster C CXX)

find_package(SDL2 REQUIRED CONFIG)

# Force C++14 compiler
set(CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 14)

#
# raycaster
#

set(SOURCES
	src/asset_store.cpp
	src/my_app.cpp
	src/screenshot.cpp
	src/sdl_app.cpp
	src/main.cpp
	)

set(HEADERS
	src/asset_store.hpp
	src/color.hpp
	src/my_app.hpp
	src/screenshot.hpp
	src/sdl_app.hpp
	)

add_executable(raycaster ${SOURCES} ${HEADERS})

target_include_directories(raycaster
	PRIVATE
		${SDL2_INCLUDE_DIRS}
	)
target_link_libraries(raycaster
	${SDL2_LIBRARIES}
	)
