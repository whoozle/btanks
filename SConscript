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
	'src/tooltip.cpp', 'src/special_zone.cpp', 'src/game_monitor.cpp', 
	'src/player_manager.cpp', 'src/object_grid.cpp', 'src/variants.cpp', 

	'ai/base.cpp', 'ai/herd.cpp', 'ai/traits.cpp',
		
	'objects/bullet.cpp', 'objects/explosion.cpp', 'objects/single_pose.cpp',
	'objects/tank.cpp', 'objects/shilka.cpp', 'objects/launcher.cpp', 'objects/ai_tank.cpp',
	'objects/ai_launcher.cpp', 'objects/ai_shilka.cpp', 'objects/ai_trooper.cpp', 'objects/ai_machinegunner_player.cpp', 
	'objects/ai_heli.cpp', 	
	'objects/traffic_lights.cpp', 'objects/missiles_in_vehicle.cpp', 'objects/missile.cpp',
	'objects/corpse.cpp', 'objects/item.cpp', 'objects/mine.cpp', 'objects/dirt.cpp', 
	'objects/damage.cpp', 'objects/helicopter.cpp', 'objects/paratrooper.cpp', 'objects/kamikaze.cpp',
	'objects/machinegunner.cpp', 'objects/destructable_object.cpp', 'objects/submarine.cpp', 'objects/train.cpp',
	'objects/trooper.cpp', 'objects/fakemod.cpp', 'objects/car.cpp', 'objects/tooltip.cpp', 
	'objects/vehicle_traits.cpp', 'objects/barrack.cpp', 'objects/watchtower.cpp',
	'objects/cannon.cpp', 'objects/boat.cpp', 'objects/poison.cpp', 'objects/old_school_destructable.cpp', 
	'objects/zombie.cpp', 'objects/civilian.cpp', 'objects/teleport.cpp', 'objects/cow.cpp', 
	'objects/heli.cpp', 'objects/bomb.cpp', 'objects/explosive.cpp', 'objects/mortar.cpp',
	
	'src/player_state.cpp', 
	'controls/joyplayer.cpp', 'controls/keyplayer.cpp', 'controls/external_control.cpp', 'controls/mouse_control.cpp', 

	'src/object.cpp', 'src/animation_model.cpp', 
	'src/resource_manager.cpp', 'src/world.cpp',
	
	'tmx/generator_object.cpp', 'tmx/tileset.cpp', 'tmx/generator.cpp', 
	'tmx/map.cpp', 'tmx/layer.cpp',
	
	'src/var.cpp', 'src/config.cpp', 
	
	'src/player_slot.cpp', 'src/hud.cpp', 'src/console.cpp',
	'src/i18n.cpp', 'src/game.cpp', 'src/window.cpp', 
	'src/credits.cpp', 'src/cheater.cpp', 

	vobj
	]
	
vorbis = 'vorbisfile'
if debug and sys.platform == "win32": 
	vorbis = 'vorbisfile_d'

#fanncxx

bt_libs = ['mrt', 'bt_sound', 'bt_net', 'bt_menu', 'sdlx',  sigc_lib, 'SDL', vorbis, al_lib, 'alut']

if sys.platform == "win32":
	bt_libs[0:0] = ['SDLmain']
	bt_libs.append('opengl32')
	bt_libs.append('user32')
	bt_libs.append('Ws2_32')
	#bt_libs.append('gdi32')
else: 
	bt_libs.append('rt')
	bt_libs.append('GL')


bt = env.SharedLibrary('bt', bt_sources, LIBS=bt_libs, RPATH=['.'])
Install('#', bt)


bt_main_sources = ['src/main.cpp']

if sys.platform == "win32":
	bt_main_sources.append('sdlx/SDL_win32_main.c')
	bt_rc = env.RES('src/bt.rc')
	bt_main_sources.append(bt_rc)


bt_main = bt_env.Program('bt', bt_main_sources, LIBS=['mrt', 'bt', 'SDL', 'user32'], RPATH=['.'])
Install('#', bt_main)
