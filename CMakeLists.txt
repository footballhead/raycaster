cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(raycaster C CXX)

find_package(SDL2 REQUIRED CONFIG)

# Force C++14 compiler
set(CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 14)

# Some SDL2 configs (*cough* Ubuntu 16.04 *cough*) require this set
cmake_policy(SET CMP0004 OLD)

add_definitions("-Wall")

#
# third-party requirements
#

add_subdirectory(ext)

#
# lua_raii
#

set(LUA_RAII_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/lua_raii/lua_raii.hpp
	)

add_library(lua_raii INTERFACE)
target_sources(lua_raii INTERFACE ${LUA_RAII_HEADERS})
target_include_directories(lua_raii INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

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
	src/sdl_application/asset_store.cpp
	src/sdl_application/input_buffer.cpp
	src/sdl_application/sdl_application.cpp
	src/sdl_application/sdl_mymath.cpp
	src/sdl_application/surface_manipulation.cpp
	)

set(SDL_APPLICATION_HEADERS
	src/sdl_application/asset_store.hpp
	src/sdl_application/input_buffer.hpp
	src/sdl_application/sdl_application.hpp
	src/sdl_application/sdl_mymath.hpp
	src/sdl_application/surface_manipulation.hpp
	)

add_library(sdl_application STATIC ${SDL_APPLICATION_SOURCES} ${SDL_APPLICATION_HEADERS})
target_include_directories(sdl_application
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${SDL2_INCLUDE_DIRS}
		mycolor
		mymath
		sdl_raii
	)

target_link_libraries(sdl_application
	${SDL2_LIBRARIES}
	mymath
	sdl_raii
	)

#
# raycaster
#

set(SOURCES
	src/raycaster/camera.cpp
	src/raycaster/intersection.cpp
	src/raycaster/level.cpp
	src/raycaster/pipeline.cpp
	src/raycaster/raycaster_app.cpp
	src/raycaster/main.cpp
	)

set(HEADERS
	src/raycaster/camera.hpp
	src/raycaster/intersection.hpp
	src/raycaster/level.hpp
	src/raycaster/pipeline.hpp
	src/raycaster/pixel_format_debug.hpp
	src/raycaster/raycaster_app.hpp
	src/raycaster/texture_cache.hpp
	)

add_executable(raycaster ${SOURCES} ${HEADERS})

set(ADDITIONAL_LIBS "")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} "pthread")
endif()

target_link_libraries(raycaster
	sdl_application
	lua
	lua_raii
	${ADDITIONAL_LIBS}
	)
