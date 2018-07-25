cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(raycaster C CXX)

# Force 64 bit for now
if(NOT "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
	message(FATAL_ERROR "Not 64 bit! Try again.")
endif()

# Yes, this is Windows-only. No, I don't care ATM.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

set(SOURCES
	src/my_app.cpp
	src/sdl_app.cpp
	src/main.cpp
	)

set(HEADERS
	src/my_app.hpp
	src/sdl_app.hpp
	)

add_executable(raycaster ${SOURCES} ${HEADERS})

# Yes, this is Windows-only. No, I don't care ATM.
target_include_directories(raycaster
	PRIVATE
		C:/SDL2-2.0.8/include
	)
target_link_libraries(raycaster
	C:/SDL2-2.0.8/lib/x64/SDL2.lib
	C:/SDL2-2.0.8/lib/x64/SDL2main.lib
	)
