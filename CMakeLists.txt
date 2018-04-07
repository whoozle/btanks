cmake_minimum_required(VERSION 2.8)
set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(MACOSX_RPATH TRUE)

include(CheckLibraryExists)
include(FindPkgConfig)

add_definitions(-DV3_DISABLE_Z -D_REENTRANT)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/clunk)

if (WIN32)
	add_definitions(-D_WINDOWS -DWIN32)
endif(WIN32)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGXX)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-long-long -pipe -pthread  -DGCC_HASCLASSVISIBILITY=1 -fvisibility=hidden -fvisibility-inlines-hidden")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-long-long -pipe -pthread")
endif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGXX)

find_package(SDL2 REQUIRED)
pkg_check_modules(SDL2IMAGE REQUIRED SDL2_image)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2IMAGE_INCLUDE_DIR})

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
