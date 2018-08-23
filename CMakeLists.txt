cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(raycaster C CXX)

find_package(SDL2 REQUIRED CONFIG)

# Force C++14 compiler
set(CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 14)

#
# mycolor
#

set(MYCOLOR_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/mycolor/mycolor.hpp
	)

add_library(mycolor INTERFACE)
target_sources(mycolor INTERFACE ${MYCOLOR_HEADERS})
target_include_directories(mycolor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#
# mymath
#

set(MYMATH_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/mymath/cartesian.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/mymath/linear_algebra.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/mymath/mymath.hpp
	)

add_library(mymath INTERFACE)
target_sources(mymath INTERFACE ${MYMATH_HEADERS})
target_include_directories(mymath INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#
# sdl_raii
#

set(SDL_RAII_SOURCES
	src/sdl_raii/sdl_app.cpp
	)

set(SDL_RAII_HEADERS
	src/sdl_raii/sdl_app.hpp
	)

add_library(sdl_raii STATIC ${SDL_RAII_SOURCES} ${SDL_RAII_HEADERS})
target_include_directories(sdl_raii
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/src
	PUBLIC
		${SDL2_INCLUDE_DIRS}
	)

#
# raycaster
#

set(SOURCES
	src/raycaster/asset_store.cpp
	src/raycaster/camera.cpp
	src/raycaster/input_buffer.cpp
	src/raycaster/my_app.cpp
	src/raycaster/screenshot.cpp
	src/raycaster/sdl_mymath.cpp
	src/raycaster/main.cpp
	)

set(HEADERS
	src/raycaster/asset_store.hpp
	src/raycaster/camera.hpp
	src/raycaster/input_buffer.hpp
	src/raycaster/my_app.hpp
	src/raycaster/screenshot.hpp
	src/raycaster/sdl_mymath.hpp
	)

add_executable(raycaster ${SOURCES} ${HEADERS})

target_include_directories(raycaster
	PRIVATE
		${SDL2_INCLUDE_DIRS}
	)
target_link_libraries(raycaster
	${SDL2_LIBRARIES}
	mymath
	sdl_raii
	)
