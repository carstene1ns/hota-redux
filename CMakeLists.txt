cmake_minimum_required(VERSION 3.22...4.3 FATAL_ERROR)

project(hota-redux VERSION 2.0.0 LANGUAGES C
	DESCRIPTION "Heart of The Alien Redux"
	HOMEPAGE_URL "")
set(PROJECT_BUGREPORT "https://github.com/carstene1ns/hota-redux/issues")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# set build type to debug, if not set
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
else()
	set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.")
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

# external libs
find_package(SDL3 3.4 REQUIRED)
find_package(SDL3_mixer REQUIRED)

add_executable(alien
	src/main.c
	src/animation.c src/animation.h
	src/audio.c src/audio.h
	src/client.c src/client.h
	src/common.c src/common.h
	src/debug.c src/debug.h
	src/decode.c src/decode.h
	src/files.c src/files.h
	src/game2bin.c src/game2bin.h
	src/lzss.c src/lzss.h
	src/parg.c src/parg.h
	src/render.c src/render.h
	src/rooms.c src/rooms.h
	src/screen.c src/screen.h
	src/sprites.c src/sprites.h
	src/vm.c src/vm.h)
target_compile_definitions(alien PRIVATE
	$<$<CONFIG:Debug>:ENABLE_DEBUG> HOTA_VERSION="${PROJECT_VERSION}")
target_compile_options(alien PRIVATE -Wall) # -Wextra

target_link_libraries(alien PRIVATE SDL3::SDL3 SDL3_mixer::SDL3_mixer)

option(WANT_ASAN "build with address sanitizer" OFF)
if(WANT_ASAN)
	message(STATUS "Building with address sanitizer")
	target_compile_options(alien PRIVATE -fno-omit-frame-pointer -fsanitize=address)
	target_link_options(alien PRIVATE -fno-omit-frame-pointer -fsanitize=address)
endif()
