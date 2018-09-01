cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(raycaster C CXX)

find_package(SDL2 REQUIRED CONFIG)

# Force C++14 compiler
set(CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 14)

# Some SDL2 configs (*cough* Ubuntu 16.04 *cough*) require this set
cmake_policy(SET CMP0004 OLD)

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
	src/sdl_raii/sdl_init.cpp
	)

set(SDL_RAII_HEADERS
	src/sdl_raii/sdl_init.hpp
	src/sdl_raii/sdl_assert.hpp
	src/sdl_raii/sdl_raii.hpp
	src/sdl_raii/sdl_wrappers.hpp
	)

add_library(sdl_raii STATIC ${SDL_RAII_SOURCES} ${SDL_RAII_HEADERS})
target_include_directories(sdl_raii
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${SDL2_INCLUDE_DIRS}
	)

#
# sdl_application
#

set(SDL_APPLICATION_SOURCES
	src/sdl_application/input_buffer.cpp
	src/sdl_application/sdl_application.cpp
	)

set(SDL_APPLICATION_HEADERS
	src/sdl_application/input_buffer.hpp
	src/sdl_application/sdl_application.hpp
	)

add_library(sdl_application STATIC ${SDL_APPLICATION_SOURCES} ${SDL_APPLICATION_HEADERS})
target_include_directories(sdl_application
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${SDL2_INCLUDE_DIRS}
		sdl_raii
	)

#
# raycaster
#

set(SOURCES
	src/raycaster/asset_store.cpp
	src/raycaster/camera.cpp
	src/raycaster/raycaster_app.cpp
	src/raycaster/screenshot.cpp
	src/raycaster/sdl_mymath.cpp
	src/raycaster/main.cpp
	)

set(HEADERS
	src/raycaster/asset_store.hpp
	src/raycaster/camera.hpp
	src/raycaster/raycaster_app.hpp
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
	sdl_application
	sdl_raii
	)
