import os, sys

#builder for static libraries
env = Environment()
if sys.platform == 'win32':
	env['SHLINKCOM'] = [env['SHLINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;2']

picLibBuilder = Builder(
		action = Action('$ARCOM'),
		emitter = '$LIBEMITTER',
		prefix = '$LIBPREFIX',
		suffix = '$LIBSUFFIX',
		src_suffix = '$OBJSUFFIX',
		src_builder = 'SharedObject')

env['BUILDERS']['StaticLibrary'] = picLibBuilder
env['BUILDERS']['Library'] = picLibBuilder

opts = Options(['options.cache'])
opts.Add('CC', 'C compiler')
opts.Add('CXX', 'C++ compiler')
opts.Add('CCFLAGS', 'General options that are passed to the C compiler', '')
opts.Add('CXXFLAGS', 'General options that are passed to the C++ compiler', '')
if sys.platform == "win32":
	opts.Add('LINK', 'Linker program', '')
opts.Add('LINKFLAGS', 'General options that are passed to the linker', '')
opts.Add('CPPPATH', 'extra cpp path', '')

if sys.platform != "win32":
	opts.Add('prefix', 'prefix for **nix packaging', '')
	opts.Add('lib_dir', 'resources directory (default: prefix/lib)', '')
	opts.Add('plugins_dir', 'plugins directory (default: prefix/lib/btanks)', '')
	opts.Add('resources_dir', 'resources directory (default: prefix/share/btanks)', '')

opts.Add(EnumOption('mode', 'build mode', 'release', allowed_values=('debug','release')))
opts.Add(BoolOption('gcc_visibility', 'gcc visibility', 'false'))
opts.Add(BoolOption('enable_lua', 'enable lua support', 'true'))

opts.Update(env)
opts.Save('options.cache', env.Clone())

Help(opts.GenerateHelpText(env))

buildmode = env['mode']
debug = buildmode == "debug"
Export('debug')

import SCons.Util
if os.environ.has_key('CC'):
	env['CC'] = os.environ['CC']
if os.environ.has_key('CFLAGS'):
	env['CCFLAGS'] += SCons.Util.CLVar(os.environ['CFLAGS'])
if os.environ.has_key('CXX'):
	env['CXX'] = os.environ['CXX']
if os.environ.has_key('CXXFLAGS'):
	env['CXXFLAGS'] += SCons.Util.CLVar(os.environ['CXXFLAGS'])
if os.environ.has_key('LDFLAGS'):
	env['LINKFLAGS'] += SCons.Util.CLVar(os.environ['LDFLAGS'])

if (env['gcc_visibility']): 
	env.Append(CCFLAGS=['-fvisibility=hidden', '-DGCC_HASCLASSVISIBILITY'])
	env.Append(CXXFLAGS=['-fvisibility-inlines-hidden', '-fvisibility=hidden', '-DGCC_HASCLASSVISIBILITY'])

if sys.platform == "win32" and debug:
	stl_port_debug = True
else: 
	stl_port_debug = False

#if stl_port_debug: 
#	env.Append(CPPDEFINES = ['_STLP_DEBUG', '_STLP_DEBUG_UNINITIALIZED'])

env.Append(CPPDEFINES = ['USE_GLSDL'])
env.Append(CPPDEFINES = ['V3_DISABLE_Z'])
env.Append(CPPDEFINES = ['_REENTRANT'])

#print sys.platform
if sys.platform == "win32":
	env.Append(CPPDEFINES = ['_WINDOWS', '_CRT_SECURE_NO_WARNINGS']) #, '_UNICODE'
	env.Append(CCFLAGS = ['/GR', '/W3', '/nologo'])
	env.Append(CXXFLAGS = ['/EHsc', '/GR', '/W3', '/nologo'])

	if debug:
		env.Append(CCFLAGS = ['/Zi', '/GT', '/Od', '/MDd', '/RTC1'])
		env.Append(CXXFLAGS = ['/Zi', '/GT', '/Od', '/MDd', '/RTC1'])
		env.Append(LINKFLAGS = ['/NOLOGO', '/DEBUG'])
	else:
		env.Append(CCCFLAGS = ['/Ot', '/Ox', '/GA', '/GF', '/Gs', '/Gy', '/MD']) #optimizations
		env.Append(CPPFLAGS = ['/Ot', '/Ox', '/GA', '/GF', '/Gs', '/Gy', '/MD']) #optimizations
		env.Append(LINKFLAGS = ['/OPT:REF', '/OPT:ICF', '/NOLOGO', '/INCREMENTAL:NO'])
		#env.Append(CPPDEFINES = ['NDEBUG']) 

else:
	if env['gcc_visibility']: 
		env.Append(CCFLAGS=['-fvisibility=hidden']);
		env.Append(CXXFLAGS=['-fvisibility-inlines-hidden', '-fvisibility=hidden']);
	if debug:
		env.Append(CCFLAGS=['-ggdb'])
		env.Append(CPPFLAGS=['-ggdb'])
	else: 
		env.Append(CCFLAGS=['-O3'])
		env.Append(CPPFLAGS=['-O3'])
		
	env.Append(CPPFLAGS=['-Wall', '-pedantic', '-Wno-long-long', '-pipe', '-pthread'])
	env.Append(CCFLAGS=['-Wall', '-pedantic', '-Wno-long-long', '-pipe', '-pthread'])


conf_env = env.Clone()
conf = Configure(conf_env)

#print conf.env['CCFLAGS']

if not conf.CheckLibWithHeader('expat', 'expat.h', 'c', "XML_ParserCreate(NULL);", False):
	Exit(1)

if not conf.CheckLibWithHeader('z', 'zlib.h', 'c', "zlibVersion();", False):
	Exit(1)

if sys.platform == "win32":
	conf.env.Append(LINKFLAGS = ['/SUBSYSTEM:WINDOWS', '/FORCE'])
	conf.env.Append(LIBS=['SDLmain'])

if not conf.CheckLibWithHeader('SDL', 'SDL/SDL.h', 'c++', "SDL_Init(0);", False):
	Exit(1)
	
if sys.platform != "win32":
	sdl_cflags = env.ParseFlags('!pkg-config --cflags sdl')
	sdl_libs = env.ParseFlags('!pkg-config --libs sdl')
	conf.env.MergeFlags(sdl_cflags, sdl_libs)
else: 
	sdl_cflags = {}
	sdl_libs = {}

Export('sdl_cflags')
Export('sdl_libs')
smpeg_lib = 'smpeg'
if debug and sys.platform == 'win32':
	smpeg_lib = 'smpeg_d'
Export('smpeg_lib')

if not conf.CheckLibWithHeader(smpeg_lib, 'smpeg/smpeg.h', 'c++', "SMPEG_new_data(malloc(42), 42, NULL, 0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('SDL_image', 'SDL/SDL_image.h', 'c++', "IMG_Load(0);", False):
	Exit(1)

if not conf.CheckLibWithHeader('vorbisfile', 'vorbis/vorbisfile.h', 'c++', "ov_open(0, 0, 0, 0);", False):
	Exit(1)

if sys.platform == 'win32' and env['enable_lua']:
	if not conf.CheckLibWithHeader('lua', 'lua.hpp', 'c++', "lua_newstate(NULL, NULL);", False):
		Exit(1)

conf.Finish()

if sys.platform == "win32":
	env.Append(LIBS=['Ws2_32', 'SDLmain'])
	env.Append(LINKFLAGS = ['/SUBSYSTEM:WINDOWS'])

if debug: 
	env.Append(CPPDEFINES = ['DEBUG'])
else:
	env.Append(CPPDEFINES = ['RELEASE'])

Export('env')
install_targets = list()
Export('install_targets')

lib_dir = '.'
try : 
	version_file = file('.svnversion', 'r')
	try : 
		version = version_file.readline().strip()
		prefix = env['prefix']
		if len(prefix): 
			env.Append(CPPDEFINES=['PREFIX="\\"' + prefix + '\\""'])
	
			if len(env['resources_dir']):
				resources_dir = env['resources_dir']
			else: 
				resources_dir = prefix + "/share/btanks"

			env.Append(CPPDEFINES=['RESOURCES_DIR="\\"' + resources_dir + '\\""'])

			if len(env['lib_dir']):
				lib_dir = env['lib_dir']
			else: 
				lib_dir = prefix + "/lib"

			if len(env['plugins_dir']):
				plugins_dir = env['plugins_dir']
			else: 
				plugins_dir = lib_dir + "/btanks"

			env.Append(CPPDEFINES='PLUGINS_DIR="\\"' + plugins_dir + '\\""')

	except: 
		info = sys.exc_info()
		print "%s %s %s" %(info[0], info[1], info[2])
except : 
	svnversion = os.popen('svnversion -n .', 'r')
	version = svnversion.readline().strip()

	if sys.platform != "win32":
		prefix = env['prefix']
		res_dir = env['resources_dir'] 
	
		if len(prefix) or len(res_dir): 	
			env.Append(CPPDEFINES=['PREFIX="\\"' + prefix + '\\""'])
	
			if len(env['resources_dir']):
				resources_dir = env['resources_dir']
			else: 
				resources_dir = prefix + "/share/btanks"

			if len(env['lib_dir']):
				lib_dir = env['lib_dir']
			else: 
				lib_dir = prefix + "/lib"

			env.Append(CPPDEFINES=['RESOURCES_DIR="\\"' + resources_dir + '\\""'])

			if len(env['plugins_dir']):
				plugins_dir = env['plugins_dir']
			else: 
				plugins_dir = lib_dir + "/btanks"

			env.Append(CPPDEFINES='PLUGINS_DIR="\\"' + plugins_dir + '\\""')

version = version[version.rfind(':') + 1:]
revision = int(version.replace('M', ''))

Export('version')
Export('revision')
Export('lib_dir')
Export('plugins_dir')

version = '0.8.%s' %version
print "version: %s" %version

bt_sublibs = ['mrt', 'sdlx', 'objects', 'clunk']
env.Append(CPPPATH=['#'])

if (os.path.exists('private')):
	dir = 'private'
	BuildDir('#/build/' + buildmode + '/' + dir, dir, 0)
	SConscript('#/build/' + buildmode + '/' + dir + '/SConscript')	

for dir in bt_sublibs:
	BuildDir('#/build/' + buildmode + '/' + dir, dir, 0)
	SConscript('#/build/' + buildmode + '/' + dir + '/SConscript')

for dir in bt_sublibs:
	env.Append(LIBPATH=['#/build/' + buildmode + '/' + dir])

env.BuildDir('#/build/' + buildmode + '/editor', 'editor', 0)
SConscript('#/build/' + buildmode + '/editor/SConscript')

env.Append(LIBPATH=['#/build/' + buildmode + '/engine'])

env.BuildDir('#/build/' + buildmode + '/engine', 'engine', 0)
SConscript('#/build/' + buildmode + '/engine/' + 'SConscript')

if len(install_targets) > 0:
	install_targets.append(env.Command(resources_dir + '/resources.dat', '#/data', 'zip -q -0 -r  $TARGET * -x \*.svn\* -x \*.wav', chdir='data'))
	env.Alias('install', install_targets)
