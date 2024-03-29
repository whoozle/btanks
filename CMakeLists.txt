cmake_minimum_required(VERSION 2.8.12)
project(btanks)
set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(MACOSX_RPATH TRUE)
set(WITH_SDL2 OFF CACHE BOOL "Force SDL1 for clunk" FORCE)

include(CheckLibraryExists)

add_definitions(-DUSE_GLSDL -DV3_DISABLE_Z -D_REENTRANT)
include_directories(${CMAKE_SOURCE_DIR})

if (WIN32)
	add_definitions(-D_WINDOWS -DWIN32)
endif(WIN32)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGXX)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-long-long -pipe -pthread  -DGCC_HASCLASSVISIBILITY=1 -fvisibility=hidden -fvisibility-inlines-hidden")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-long-long -pipe -pthread")
endif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGXX)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDLIMAGE_INCLUDE_DIR})

check_library_exists(smpeg SMPEG_new_data "" SMPEG_FOUND)
check_library_exists(vorbisfile ov_open "" VORBISFILE_FOUND)

add_subdirectory(mrt)
add_subdirectory(sdlx)
add_subdirectory(clunk)
add_subdirectory(engine)
add_subdirectory(objects)
add_subdirectory(editor)

set(btanks_SOURCES engine/src/main.cpp)
if (WIN32)
	list(APPEND btanks_SOURCES engine/src/bt.rc)
endif(WIN32)

include_directories(engine)
add_executable(btanks ${btanks_SOURCES})
target_link_libraries(btanks mrt sdlx btanks_engine)
add_dependencies(btanks bt_objects)

if (WIN32)
	target_link_libraries(btanks SDL sdlx_main user32)
else(WIN32)
	target_link_libraries(btanks ${SDLMAIN_LIBRARY})
endif(WIN32)

if (EXISTS "${CMAKE_SOURCE_DIR}/private")
	add_subdirectory(private)
endif()
