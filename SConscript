import sys

Import('env')
Import('version')
Import('revision')
Import('debug')
Import('sigc_lib')
Import('al_lib')

env = env.Copy()
venv = env.Copy()
bt_env = env.Copy()

venv.Append(CPPDEFINES=['VERSION="\\"' + version + '\\""'])
venv.Append(CPPDEFINES=['REVISION=%d' % revision])

venv.Append(CPPDEFINES=['BTANKSAPI=DLLEXPORT']);
env.Append(CPPDEFINES=['BTANKSAPI=DLLEXPORT']);

vobj = venv.SharedObject('src/version.cpp')
bt_sources = 	[
	'src/finder.cpp', 'src/zbox.cpp', 
	
	'src/alarm.cpp', 'src/base_object.cpp', 'src/notifying_xml_parser.cpp',
	'src/special_zone.cpp', 'src/game_monitor.cpp', 
	'src/player_manager.cpp', 'src/object_grid.cpp', 'src/variants.cpp', 

	'ai/base.cpp', 'ai/herd.cpp', 'ai/traits.cpp', 'ai/waypoints.cpp', 
		
	'src/player_state.cpp', 'controls/joy_bindings.cpp', 
	'controls/joyplayer.cpp', 'controls/keyplayer.cpp', 'controls/external_control.cpp', 'controls/mouse_control.cpp', 

	'src/object.cpp', 'src/animation_model.cpp', 
	'src/resource_manager.cpp', 'src/world.cpp',
	
	'tmx/generator_object.cpp', 'tmx/tileset.cpp', 'tmx/generator.cpp', 'tmx/tileset_list.cpp',  
	'tmx/map.cpp', 'tmx/layer.cpp',
	
	'src/var.cpp', 'src/config.cpp', 
	
	'src/player_slot.cpp', 'src/hud.cpp', 'src/console.cpp',
	'src/i18n.cpp', 'src/game.cpp', 'src/window.cpp', 
	'src/credits.cpp', 'src/cheater.cpp', 
	'src/vehicle_traits.cpp', 
	
	'sound/al_ex.cpp', 'sound/mixer.cpp', 'sound/ogg_ex.cpp', 'sound/ogg_stream.cpp', 'sound/sample.cpp',

	vobj
	]
	
vorbis = 'vorbisfile'
if debug and sys.platform == "win32": 
	vorbis = 'vorbisfile_d'

#fanncxx

bt_libs = ['mrt', 'bt_net', 'bt_menu', 'sdlx',  sigc_lib, 'SDL', vorbis, al_lib]

if sys.platform == "win32":
	bt_libs[0:0] = ['SDLmain']
	bt_libs.append('opengl32')
	bt_libs.append('Ws2_32')
	bt_libs.append('user32')
	#bt_libs.append('gdi32')
else: 
	bt_libs.append('GL')


bt = env.SharedLibrary('bt', bt_sources, LIBS=bt_libs, RPATH=['.'])
Install('#', bt)


bt_main_sources = ['src/main.cpp']
bt_main_libs =  ['mrt', 'bt', 'bt_objects', 'SDL', 'sdlx']

if sys.platform == "win32":
	bt_main_sources.append('sdlx/SDL_win32_main.c')
	bt_rc = env.RES('src/bt.rc')
	bt_main_sources.append(bt_rc)
	bt_main_libs.append('user32')


if sys.platform != 'win32':
	bt_env.Append(LINKFLAGS=' -Wl,-rpath . -Wl,-rpath-link build/' + env['mode'] + '/mrt')
	bt_env.Append(LINKFLAGS=' -Wl,-rpath . -Wl,-rpath-link build/' + env['mode'] + '/sdlx')
	bt_env.Append(LINKFLAGS=' -Wl,-rpath . -Wl,-rpath-link build/' + env['mode'] + '/objects')

bt_main = bt_env.Program('bt', bt_main_sources, LIBS=bt_main_libs, RPATH=['.'])
Install('#', bt_main)
