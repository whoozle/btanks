import os, sys

env = Environment()

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
	opts.Add('prefix', 'prefix for **nix packaging', '/usr/local')
opts.Add(EnumOption('mode', 'build mode', 'release', allowed_values=('debug','release')))

opts.Update(env)
opts.Save('options.cache', env.Copy())

Help(opts.GenerateHelpText(env))

buildmode = env['mode']
debug = buildmode == "debug"
Export('debug')


if sys.platform == "win32" and debug:
	stl_port_debug = True
else: 
	stl_port_debug = False

if stl_port_debug: 
	env.Append(CPPDEFINES = ['_STLP_DEBUG', '_STLP_DEBUG_UNINITIALIZED'])

env.Append(CPPDEFINES = ['USE_GLSDL'])
env.Append(CPPDEFINES = ['V3_DISABLE_Z'])
env.Append(CPPDEFINES = ['_REENTRANT'])

#print sys.platform
if sys.platform == "win32":
	al_lib = 'openal32'
	env.Append(CPPDEFINES = ['WIN32', '_WINDOWS']) #, '_UNICODE'
	env.Append(CCFLAGS = ' /GR /W3 /nologo ')
	env.Append(CPPFLAGS = ' /GX /GR /W3 /nologo ')

	if debug:
		env.Append(CCFLAGS = ' /Yd /Zi /Ge /GT /GZ /Od /ZI /MDd ')
		env.Append(CPPFLAGS = ' /Yd /Zi /Ge /GT /GZ /Od /ZI /MDd ')
		env.Append(LINKFLAGS = ' /NOLOGO /DEBUG ')
	else:
		env.Append(CCCFLAGS = ' /Ot /Ob2gity /G6 /GA /GF /Gs /Gy /MD ') #optimizations
		env.Append(CPPFLAGS = ' /Ot /Ob2gity /G6 /GA /GF /Gs /Gy /MD ') #optimizations
		env.Append(LINKFLAGS = ' /OPT:REF /OPT:ICF /NOLOGO /INCREMENTAL:NO ')
		#env.Append(CPPDEFINES = ['NDEBUG']) 

#	
#	env.Append(CPPFLAGS = '/Ox /Ot ') #optimizations
#	env.Prepend(CPPPATH=' C:\\\\STLport-4.6.2\\\\stlport ')
else:
	if debug:
		env.Append(CCFLAGS='-ggdb ')
		env.Append(CPPFLAGS='-ggdb ')
	else: 
		env.Append(CCFLAGS='-O3 ')
		env.Append(CPPFLAGS='-O3 ')
		
	al_lib = 'openal'
	env.Append(CPPFLAGS=' -Wall -pedantic -Wno-long-long -pipe ')
	env.Append(CCFLAGS=' -Wall -pedantic -Wno-long-long -pipe ')


conf_env = env.Copy()
conf = Configure(conf_env)

os.environ['PKG_CONFIG_PATH'] = "/usr/local/lib/pkgconfig/"

if sys.platform != "win32":
	sigc_p = os.popen('pkg-config --cflags sigc++-2.0', 'r')
	sigc_flags = sigc_p.readline().strip()
	sigc_p = os.popen('pkg-config --libs-only-L sigc++-2.0', 'r')
	sigc_lflags = [sigc_p.readline().strip()]
	conf_env.Append(LINK_FLAGS=sigc_lflags)
	env.Append(LINK_FLAGS=sigc_lflags)
	sigc_lib = 'sigc-2.0' #guess 
	
else: 
	sigc_flags = ''
	if stl_port_debug:
		sigc_lib = 'sigc-2.0d'
	else: 
		sigc_lib = 'sigc-2.0'

conf_env.Append(CXXFLAGS=sigc_flags)


#print conf.env['CCFLAGS']


if not conf.CheckLibWithHeader(sigc_lib, 'sigc++/sigc++.h', 'c++', "sigc::signal1<int,int> sig;", False):
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

if not conf.CheckLibWithHeader(al_lib, 'AL/al.h', 'c++', "ALuint s; alGenSources(1, &s);", False):
	Exit(1)

if not conf.CheckLibWithHeader('vorbisfile', 'vorbis/vorbisfile.h', 'c++', "ov_open(0, 0, 0, 0);", False):
	Exit(1)

conf.env.Append(LIBS=al_lib)
if not conf.CheckLibWithHeader('alut', 'AL/alut.h', 'c++', "alutInit(0,0);", False):
	Exit(1)

#if not conf.CheckLibWithHeader('fann', 'fann.h', 'c', "fann_create_standard_array(0, 0);", False):
#	Exit(1)

#if not conf.CheckLibWithHeader('SDL_net', 'SDL/SDL_net.h', 'c++', "SDLNet_Init();", False):
#	Exit(1)

conf.Finish()

if sys.platform == "win32":
	env.Append(LIBS=['Ws2_32', 'SDLmain'])
	env.Append(LINKFLAGS = '/SUBSYSTEM:WINDOWS ')

if debug: 
	env.Append(CPPDEFINES = ['DEBUG'])
else:
	env.Append(CPPDEFINES = ['RELEASE'])

Export('env')
Export('sigc_flags')
Export('sigc_lib')
Export('al_lib')

try : 
	version_file = file('.svnversion', 'r')
	version = version_file.readline().strip()
	env.Append(CPPDEFINES='PREFIX="' + env['prefix'] + '"')
except : 
	svnversion = os.popen('svnversion -n .', 'r')
	version = svnversion.readline().strip()

version = version[version.rfind(':') + 1:]
revision = int(version.replace('M', ''))
	
Export('version')
Export('revision')

version = '0.4.%s' %version
print "version: %s" %version


env.Append(CPPPATH=['.', 'src'])
env.Append(CXXFLAGS=sigc_flags)

env.Append(CPPPATH=['#', '#/src'])

bt_sublibs = ['mrt', 'sdlx', 'net', 'menu', 'sound', 'ai'] #fann

for dir in bt_sublibs:
	BuildDir('#/build/' + buildmode + '/' + dir, dir, 0)
	SConscript('#/build/' + buildmode + '/' + dir + '/SConscript')

for dir in bt_sublibs:
	env.Append(LIBPATH=['#/build/' + buildmode + '/' + dir])

env.BuildDir('#/build/' + buildmode, '#', 0)
SConscript('#/build/' + buildmode + '/' + 'SConscript')
