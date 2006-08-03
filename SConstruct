import sys
import os

env = Environment()

opts = Options(['options.cache'])
opts.Add('CC', 'C compiler')
opts.Add('CXX', 'C++ compiler')
opts.Add('CCFLAGS', 'General options that are passed to the C compiler', '')
opts.Add('CXXFLAGS', 'General options that are passed to the C++ compiler', '')

opts.Update(env)
opts.Save('options.cache', env.Copy())

Help(opts.GenerateHelpText(env))


#print sys.platform
if sys.platform == "win32":
	env.Append(CPPDEFINES = ['WIN32', '_WINDOWS']) #, '_UNICODE'
	env.Append(CCFLAGS = '/GX /GR /W3 /MT ')
	env.Append(CPPFLAGS = '/GX /GR /W3 /MT ')
#	env.Append(LINKFLAGS = '/OPT:NOREF /OPT:NOICF /INCREMENTAL:NO')
	env.Append(CCFLAGS = '/Ox /Ot ') #optimizations
	env.Append(CPPFLAGS = '/Ox /Ot ') #optimizations
else:
	env.Append(CPPFLAGS=' -Wall -pedantic -ggdb3 -Wno-long-long -pipe ')
	env.Append(CCFLAGS=' -Wall -pedantic -ggdb3 -Wno-long-long -pipe ')


conf_env = env.Copy()
conf = Configure(conf_env)

sigc_cpppath = ['/usr/lib/sigc++-2.0/include', '/usr/include/sigc++-2.0', '/usr/local/include/sigc++-2.0', '/usr/local/lib/sigc++-2.0/include']
sigc_lib = 'sigc-2.0'

conf_env.Prepend(CPPPATH=sigc_cpppath)

#print conf.env['CCFLAGS']


if not conf.CheckLibWithHeader(sigc_lib, 'sigc++/sigc++.h', 'c++', "SigC::Signal1<int,int> sig;", False):
	Exit(1)

if not conf.CheckLibWithHeader('expat', 'expat.h', 'c', "XML_ParserCreate(NULL);", False):
	Exit(1)

if not conf.CheckLibWithHeader('z', 'zlib.h', 'c', "zlibVersion();", False):
	Exit(1)

if sys.platform == "win32":
	conf.env.Append(LINKFLAGS = '/SUBSYSTEM:WINDOWS /FORCE')
	conf.env.Append(LIBS=['SDLmain'])

if not conf.CheckLibWithHeader('SDL', 'SDL/SDL.h', 'c++', "SDL_Init(0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_image', 'SDL/SDL_image.h', 'c++', "IMG_Load(0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_ttf', 'SDL/SDL_ttf.h', 'c++', "TTF_Init();", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_net', 'SDL/SDL_net.h', 'c++', "SDLNet_Init();", False):
	Exit(1)

conf.Finish()

if sys.platform == "win32":
	env.Append(LINKFLAGS = '/SUBSYSTEM:WINDOWS /FORCE')
	env.Append(LIBS=['SDLmain'])


Export('env')
Export('sigc_cpppath')
Export('sigc_lib')

SConscript('mrt/SConscript')
SConscript('sdlx/SConscript')

env = env.Copy()
env.Append(LIBPATH=['mrt', 'sdlx', 'src'])
env.Prepend(CPPPATH=['.', 'src'])
env.Append(CPPPATH=sigc_cpppath)

svnversion = os.popen('svnversion -n .', 'r')
version = svnversion.readline()
version = version[version.rfind(':') + 1:]
print "version: %s" %version

venv = env.Copy()
venv.Append(CPPDEFINES=['VERSION="\\"' + version + '\\""'])

vobj = venv.Object('src/version.cpp')

bt = env.Program('bt', 
	['src/alarm.cpp', 'src/object.cpp', 
	
	'objects/bullet.cpp', 'objects/explosion.cpp', 'objects/corpse.cpp',
	'objects/joyplayer.cpp', 'objects/keyplayer.cpp', 'objects/player.cpp', 'objects/aiplayer.cpp',
	
	'net/protocol.cpp', 'net/server.cpp', 'net/client.cpp', 'net/connection.cpp',

	'src/menu.cpp', 'src/menuitem.cpp', 
	'src/animated_object.cpp', 'src/animation_model.cpp', 
	'src/resource_manager.cpp', 'src/world.cpp',
	'tmx/map.cpp', 'tmx/layer.cpp', 
	'sdl_collide/SDL_collide.c', 
	'src/main.cpp', 'src/game.cpp', 
	vobj
	], 
	
	LIBS=['sdlx', 'mrt', sigc_lib, 'SDL_gfx', 'SDL_ttf', 'SDL_image', 'SDL_net', 'SDL', 'expat', 'z'], RPATH=['.'])
