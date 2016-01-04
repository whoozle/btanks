set(bted_SOURCES
	base_brush.cpp
	editor.cpp
	open_map_dialog.cpp
	tileset_dialog.cpp
	layer_item.cpp
	layer_list_dialog.cpp
	command.cpp
	add_tileset_dialog.cpp
	add_object_dialog.cpp
	object_properties.cpp
	tilebox_brush.cpp
	morph_dialog.cpp
	resize_dialog.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/engine)
include_directories(${CMAKE_SOURCE_DIR}/engine/src)

add_executable(bted ${bted_SOURCES})
target_link_libraries(bted mrt sdlx btanks_engine)
add_dependencies(bted bt_objects)

get_property(BTED_LOCATION TARGET bted PROPERTY LOCATION)
add_custom_command(TARGET bted POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ARGS ${BTED_LOCATION} ${CMAKE_BINARY_DIR})

if (WIN32)
	target_link_libraries(bted SDL sdlx_main user32)
else(WIN32)
	target_link_libraries(bted ${SDLMAIN_LIBRARY})
endif(WIN32)
