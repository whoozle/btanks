include_directories(. src)

add_definitions(-DBTANKSAPI=DLLEXPORT)

set(bt_SOURCES
#networking
	net/message.cpp net/server.cpp net/client.cpp net/connection.cpp net/monitor.cpp net/scanner.cpp
#menu
	menu/checkbox.cpp menu/shop.cpp menu/image_view.cpp menu/shop_item.cpp menu/popup_menu.cpp
	menu/start_server_menu.cpp menu/join_server_menu.cpp menu/campaign_menu.cpp menu/chat.cpp
	menu/menu.cpp menu/box.cpp menu/upper_box.cpp menu/container.cpp menu/menu_item.cpp
	menu/map_picker.cpp menu/scroll_list.cpp menu/control.cpp menu/map_details.cpp menu/player_picker.cpp
	menu/chooser.cpp menu/label.cpp menu/button.cpp menu/menu_config.cpp menu/map_desc.cpp
	menu/prompt.cpp menu/text_control.cpp menu/host_list.cpp menu/options_menu.cpp menu/control_picker.cpp
	menu/slider.cpp menu/redefine_keys.cpp menu/tooltip.cpp menu/player_name_control.cpp
	menu/host_item.cpp menu/number_control.cpp menu/video_control.cpp menu/network_status.cpp
	menu/join_team.cpp menu/mode_panel.cpp menu/grid.cpp menu/notepad.cpp menu/video_control_disabled.cpp
	menu/simple_gamepad_setup.cpp menu/medals.cpp menu/image.cpp menu/main_menu.cpp menu/profiles_menu.cpp
	menu/new_profile_dialog.cpp
#end of menu

	src/finder.cpp src/zbox.cpp src/campaign.cpp src/netstats.cpp

	src/alarm.cpp src/base_object.cpp src/notifying_xml_parser.cpp src/xml_parser.cpp
	src/special_zone.cpp src/game_monitor.cpp
	src/player_manager.cpp src/variants.cpp

	ai/base.cpp ai/buratino.cpp ai/herd.cpp ai/traits.cpp ai/waypoints.cpp ai/rush.cpp ai/old_school.cpp ai/trooper.cpp ai/targets.cpp

	src/player_state.cpp controls/control_method.cpp controls/simple_joy_bindings.cpp
	controls/joyplayer.cpp controls/keyplayer.cpp controls/mouse_control.cpp

	src/object.cpp src/animation_model.cpp src/rotating_object.cpp
	src/resource_manager.cpp src/world.cpp

	tmx/generator_object.cpp tmx/tileset.cpp tmx/generator.cpp tmx/tileset_list.cpp
	tmx/map.cpp tmx/layer.cpp

	src/var.cpp src/config.cpp src/registrar.cpp src/profiler.cpp

	src/player_slot.cpp src/hud.cpp src/console.cpp
	src/i18n.cpp src/game.cpp src/window.cpp src/logo.cpp
	src/credits.cpp src/cheater.cpp
	src/vehicle_traits.cpp src/nickname.cpp
	src/rt_config.cpp src/team.cpp

	sound/mixer.cpp sound/ogg_ex.cpp sound/ogg_stream.cpp

	src/version.cpp
)

set_source_files_properties(src/version.cpp PROPERTIES COMPILE_DEFINITIONS "VERSION=\"git\";REVISION=9999")

#lua support
add_definitions(-DENABLE_LUA)
list(APPEND bt_SOURCES luaxx/state.cpp luaxx/lua_hooks.cpp)

add_library(btanks_engine SHARED ${bt_SOURCES})
target_link_libraries(btanks_engine vorbisfile SDL smpeg mrt sdlx vorbis clunk ${LUA_LIBRARY})

if (WIN32)
	target_link_libraries(btanks_engine Ws2_32 user32)
endif (WIN32)
