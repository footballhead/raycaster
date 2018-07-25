# Windows SDL raycaster sample

## Requirements

 * 64 bit everything. This only compiles for 64 bit machines.
 * Visual Studio 2017 Community Edition with Desktop Development package
 * SDL 2.0.8 (installed to C:/SDL-2.0.8/)
 * CMake 3.11

## Building

Open VS x64 terminal

	mkdir build
	cd build
	cmake -G "Visual Studio 15 2017 Win64" ..
	msbuild raycaster.sln

## Running

Copy SDL.dll into `build/Debug`. Then, from repo root (NOT `build/`!!), run:

	build\Debug\raycaster.exe
