include_directories(.)
include_directories(../mrt)
add_definitions(-DSDLXAPI=DLLEXPORT)

set(sdlx_SOURCES 
	cursor.cpp sdl_ex.cpp surface.cpp system.cpp joystick.cpp
	thread.cpp mutex.cpp semaphore.cpp color.cpp
	c_map.cpp font.cpp timer.cpp module.cpp file_rw.cpp
	gfx/SDL_rotozoom.c
)

if (WIN32)
	list(APPEND sdlx_SOURCES wrappers/d3dsdl8.cpp)
	include_directory("$ENV{DXSDK_DIR}\\Include")
	set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH} $ENV{DXSDK_DIR}\\Lib")
else(WIN32)
	list(APPEND sdlx_SOURCES wrappers/glSDL.c)
endif(WIN32)

add_library(sdlx SHARED ${sdlx_SOURCES})
target_link_libraries(sdlx mrt ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})

if (WIN32)
	target_link_libraries(sdlx d3d8 d3dx8 winmm user32 gdi32 advapi32)
	add_library(sdlx_main STATIC SDL_win32_main.c)
else (WIN32)
	target_link_libraries(sdlx rt X11)
endif (WIN32)


