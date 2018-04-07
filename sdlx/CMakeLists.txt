include_directories(.)
include_directories(../mrt)
add_definitions(-DSDLXAPI=DLLEXPORT)

set(sdlx_SOURCES 
	cursor.cpp sdl_ex.cpp surface.cpp system.cpp joystick.cpp
	thread.cpp mutex.cpp semaphore.cpp color.cpp
	c_map.cpp font.cpp timer.cpp module.cpp file_rw.cpp
)

add_library(sdlx SHARED ${sdlx_SOURCES})
target_link_libraries(sdlx mrt ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARY})

target_link_libraries(sdlx rt X11)


