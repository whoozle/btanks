cmake_minimum_required(VERSION 2.8)
include(CheckLibraryExists)

add_definitions(-DUSE_GLSDL -DV3_DISABLE_Z -D_REENTRANT)
include_directories(.)

if (WIN32)
	add_definitions(-D_WINDOWS -DWIN32)
endif(WIN32)

if (${CMAKE_COMPILER_IS_GNUCC})
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -D_FORTIFY_SOURCE=2")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -D_FORTIFY_SOURCE=2")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-long-long -pipe -pthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-long-long -pipe -pthread")
endif (${CMAKE_COMPILER_IS_GNUCC})

find_package(Lua51)

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

set(btanks_SOURCES engine/src/main.cpp)
if (WIN32)
	list(APPEND btanks_SOURCES engine/src/bt.rc)
endif(WIN32)

include_directories(engine)
add_executable(btanks ${btanks_SOURCES})
target_link_libraries(btanks mrt sdlx btanks_engine)

if (WIN32)
	target_link_libraries(btanks SDL sdlx_main user32)
else(WIN32)
	target_link_libraries(btanks ${SDLMAIN_LIBRARY})
endif(WIN32)
