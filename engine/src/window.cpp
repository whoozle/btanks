/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef _WINDOWS
#	include <windows.h>
#endif


#include "window.h"
#include "config.h"
#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "version.h"
#include "finder.h"
#include <stdlib.h>
#include <assert.h>

#ifdef _WINDOWS
#	define putenv _putenv

static STICKYKEYS g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
static TOGGLEKEYS g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
static FILTERKEYS g_StartupFilterKeys = {sizeof(FILTERKEYS), 0};    

void AllowAccessibilityShortcutKeys( bool bAllowKeys )
{
    if( bAllowKeys )
    {
        // Restore StickyKeys/etc to original state and enable Windows key      
        STICKYKEYS sk = g_StartupStickyKeys;
        TOGGLEKEYS tk = g_StartupToggleKeys;
        FILTERKEYS fk = g_StartupFilterKeys;
        
        SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
        SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
        SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
    }
    else
    {
        // Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
        // then leave the settings alone as its probably being usefully used
 
        STICKYKEYS skOff = g_StartupStickyKeys;
        if( (skOff.dwFlags & SKF_STICKYKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
            skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
        }
 
        TOGGLEKEYS tkOff = g_StartupToggleKeys;
        if( (tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
            tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
        }
 
        FILTERKEYS fkOff = g_StartupFilterKeys;
        if( (fkOff.dwFlags & FKF_FILTERKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
            fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
        }
    }
}
#else 

#include <SDL_opengl.h>
#	ifndef SDL_OPENGLBLIT
#		define SDL_OPENGLBLIT 0
// using 0 as OPENGLBLIT value. SDL 1.3 or later
#	endif

#endif


IMPLEMENT_SINGLETON(Window, IWindow);

IWindow::IWindow() : _fr(10.0f) {}

void IWindow::initSDL() {
	//putenv(strdup("SDL_VIDEO_WINDOW_POS"));
	putenv(strdup("SDL_VIDEO_CENTERED=1"));

#ifdef _WINDOWS
	LOG_DEBUG(("direct3d: %s, vsync: %s", _dx?"yes":"no", _vsync?"yes":"no"));
	putenv(strdup("SDL_VIDEO_RENDERER=gdi"));

	if (_dx) 
#	if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
		_putenv(strdup("SDL_VIDEO_RENDERER=d3d"));
#	else
		//_putenv(strdup("SDL_VIDEODRIVER=directx"));
#	endif

#else 
	LOG_DEBUG(("gl: %s, vsync: %s", _opengl?"yes":"no", _vsync?"yes":"no"));
#endif

//opengl renderer
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
	if (_opengl)
		_putenv(strdup("SDL_VIDEO_RENDERER=opengl"));
#endif

	LOG_DEBUG(("initializing SDL..."));
	Uint32 subsystems = SDL_INIT_VIDEO | SDL_INIT_TIMER | (_init_joystick? SDL_INIT_JOYSTICK: 0);
#ifdef DEBUG
	sdlx::System::init(subsystems | SDL_INIT_NOPARACHUTE);
#else
	sdlx::System::init(subsystems);
#endif
	{
		SDL_version compiled;
		SDL_VERSION(&compiled);
		const SDL_version *linked = SDL_Linked_Version();
		assert(linked != NULL); //paranoid, 1.2 SDL got return &static_version; there.
		LOG_DEBUG(("compiled version: %u.%u.%u, linked: %u.%u.%u", 
			compiled.major, compiled.minor, compiled.patch, 
			linked->major, linked->minor, linked->patch
		));
		
		if (compiled.major != linked->major || 
			compiled.minor != linked->minor || 
			compiled.patch != linked->patch) {
			LOG_WARN(("Engine was compiled with different version of SDL library. Do not report any bugs then!"));
		}
	}
	
	LOG_DEBUG(("enabling unicode..."));
	SDL_EnableUNICODE(1);
	
	LOG_DEBUG(("turning on keyboard repeat..."));
	if (SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) == -1)
		LOG_ERROR(("SDL_EnableKeyRepeat failed: %s", SDL_GetError()));

	int default_flags = sdlx::Surface::Hardware | sdlx::Surface::Alpha;

#ifndef _WINDOWS
	default_flags |= (_opengl? SDL_OPENGL: 0);

	if (_opengl) {
		LOG_DEBUG(("loading GL library"));
		int r = SDL_GL_LoadLibrary(NULL);
		if (r == -1) {
			LOG_WARN(("SDL_GL_LoadLibrary failed: %s", SDL_GetError()));
			_opengl = false;
		}
	}

#endif
	
#ifdef _WINDOWS
	if (_dx) {
#else
	if (_opengl) {
#endif

#ifdef USE_GLSDL
		default_flags &= ~SDL_OPENGL;
		default_flags |= SDL_GLSDL;
#endif
	}

	sdlx::Surface::set_default_flags(default_flags);

	//LOG_DEBUG(("initializing SDL_ttf..."));
	//sdlx::TTF::init();
}

#if !defined _WINDOWS and !defined __APPLE__
static std::string getGLString(const GLenum name) {
	typedef const GLubyte * (APIENTRY * PGLGETSTRING) (GLenum);
	union {
		PGLGETSTRING pglGetString;
		void *ptr;
	} ptr_hack;

	memset(&ptr_hack, 0, sizeof(ptr_hack));
	
	ptr_hack.ptr = SDL_GL_GetProcAddress("glGetString");
	if (ptr_hack.pglGetString) {
		const char * cstr = (const char *) ptr_hack.pglGetString(name);
		if (cstr != NULL) {
			return cstr;
		} else {
			LOG_WARN(("could not get value for GLenum %d.", (int)name));
		}
	} else LOG_WARN(("glGetString not found."));
	return std::string();
}

#endif

#include "mrt/chunk.h"

void IWindow::init(const int argc, char *argv[]) {
#ifdef _WINDOWS
	{
		GET_CONFIG_VALUE("engine.use-high-priority-class", bool, uhpc, true);
		if (uhpc) {
			LOG_DEBUG(("setting high priority class..."));
			HANDLE pid = GetCurrentProcess();
			if (SetPriorityClass(pid, HIGH_PRIORITY_CLASS) == FALSE)
				LOG_WARN(("SetPriorityClass(HIGH_PRIORITY_CLASS) failed. nevermind, using default."));
		}
	}
#endif

	_init_joystick = true;
	
	_fullscreen = false;
	_vsync = false;
	_fsaa = 0;

#ifndef _WINDOWS
	_opengl = true;
	_force_soft = false;
#else
	_dx = true;
#endif

	bool force_gl = false;
	Config->get("engine.window.width", _w, 800);
	Config->get("engine.window.height", _h, 600);
	Config->get("engine.window.fullscreen", _fullscreen, false);
	
	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--fs") == 0) _fullscreen = true;
#ifndef _WINDOWS
		else if (strcmp(argv[i], "--no-gl") == 0) _opengl = false;
		else if (strcmp(argv[i], "--force-gl") == 0) { force_gl = true; }
		else if (strcmp(argv[i], "--force-soft-gl") == 0) { _force_soft = true; }
#else
		else if (strcmp(argv[i], "--no-dx") == 0) { _dx = false; }
		else if (strcmp(argv[i], "--no-gl") == 0) { _dx = false; }
#endif
		else if (strcmp(argv[i], "--vsync") == 0) _vsync = true;
		//else if (strcmp(argv[i], "--320x200") == 0) { _w = 320; _h = 200; }
		//else if (strcmp(argv[i], "--320x240") == 0) { _w = 320; _h = 240; }
		else if (strcmp(argv[i], "-0") == 0) { _w = 640; _h = 480; }
		else if (strcmp(argv[i], "-1") == 0) { _w = 800; _h = 600; }
		else if (strcmp(argv[i], "-2") == 0) { _w = 1024; _h = 768; }
		else if (strcmp(argv[i], "-3") == 0) { _w = 1152; _h = 864; }
		else if (strcmp(argv[i], "-4") == 0) { _w = 1280; _h = 1024; }
		else if (strcmp(argv[i], "--fsaa") == 0) { _fsaa = (_fsaa)?(_fsaa<< 1) : 1; }
		else if (strcmp(argv[i], "--no-joystick") == 0) { _init_joystick = false; }
		else if (strcmp(argv[i], "--help") == 0) { 
			printf(
					"\t--no-gl\t\t\tdisable GL renderer (linux only/custom non-d3d builds)\n"
					"\t--no-dx\t\t\tdisable D3D renderer (windows only)\n"
					"\t-2 -3 -4\t\t\tenlarge video mode to 1024x768, 1152x864 or 1280x1024\n"
				  );
			return;
		}
	}
	
	initSDL();

#if 0
#ifdef _WINDOWS
	LOG_DEBUG(("loading icon..."));
	TRY {
		HANDLE h = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
		if (h == NULL)
			throw_ex(("LoadImage failed"));
		ULONG r = SetClassLongPtr(NULL, GCLP_HICON, (LONG_PTR)h);
		LOG_DEBUG(("SetClassLongPtr returned %08lx", (unsigned long)r));

	} CATCH("icon setup", );
#endif
#endif

LOG_DEBUG(("setting caption..."));		
SDL_WM_SetCaption(("Battle tanks - " + getVersion()).c_str(), "btanks");

#if !defined _WINDOWS && !defined __APPLE__
	TRY {
		mrt::Chunk data;
		Finder->load(data, "tiles/icon.png");

		sdlx::Surface icon;
		icon.load_image(data);
		SDL_WM_SetIcon(icon.get_sdl_surface(), NULL);
	} CATCH("setting icon", {});

#endif

	if (_opengl && !force_gl && !sdlx::System::accelerated_gl(!_fullscreen)) {
		LOG_WARN(("could not find accelerated GL, falling back to software mode"));
		_opengl = false;
	}

	createMainWindow();

#ifdef _WINDOWS
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
	SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
	SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
	
	AllowAccessibilityShortcutKeys(false);
#endif
}

static int temp_nod(int a, int b) {
	int p = a % b;
	while (p != 0) {
		a = b; 
		b = p;
		p = a % b;
	}
	return b;
}

void IWindow::createMainWindow() {
	//Config->get("engine.window.width", _w, 800);
	//Config->get("engine.window.height", _h, 600);

	int flags = SDL_HWSURFACE | SDL_ANYFORMAT;
	flags |= SDL_DOUBLEBUF;
	
	if (_fullscreen) flags |= SDL_FULLSCREEN;

	TRY {
		SDL_Rect **modes;
		/* Get available fullscreen/hardware modes */
		modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

		/* Check is there are any modes available */
		if(modes == (SDL_Rect **)0) 
			throw_ex(("No video modes available"));
    
	    /* Check if our resolution is restricted */
    	if(modes == (SDL_Rect **)-1){
			LOG_DEBUG(("all resolutions available."));
		} else {
			/* Print valid modes */
			LOG_DEBUG(("available modes:"));
			for(int i = 0; modes[i] != NULL; ++i) {
				int w = modes[i]->w, h = modes[i]->h;
				if (w < 800 || h < 600)
					continue;
				int nod = temp_nod(w, h);
				int aw = w / nod, ah = h / nod;
				if (w > 800 && w < 1024 && aw == 4 && ah == 3)
					continue; //skip numerous stupid modes. disable this if you want to
				LOG_DEBUG(("\t%dx%d, %d:%d", w, h, aw, ah));
				resolutions.push_front(*modes[i]);
			}
		}
	} CATCH("screen modes probe", );
	
#ifndef _WINDOWS
	if (_opengl) {
#if SDL_VERSION_ATLEAST(1,2,10)
		LOG_DEBUG(("setting GL swap control to %d...", _vsync?1:0));
		int r = SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, _vsync?1:0);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_SWAP_CONTROL."));

		if (_vsync)
			SDL_putenv(strdup("__GL_SYNC_TO_VBLANK=1")); //nvidia ext
			
#if 0
		if (!_vsync) {
			typedef void (APIENTRY * WGLSWAPINTERVALEXT) (int);
			WGLSWAPINTERVALEXT wglSwapIntervalEXT = (WGLSWAPINTERVALEXT) SDL_GL_GetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT) {
				LOG_DEBUG(("disabling vsync with SwapIntervalEXT(0)..."));
			    wglSwapIntervalEXT(0); // disable vertical synchronisation
			} else LOG_WARN(("wglSwapIntervalEXT not found. vsync option will not have any effect"));
		}
#endif

#ifdef _WINDOWS
		LOG_DEBUG(("setting GL accelerated visual..."));

		//SIGSEGV in SDL under linux if no GLX visual present. (debian sid, fc6)
		r = SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_ACCELERATED_VISUAL."));
#endif

#endif
		
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		
		if (_fsaa > 0) {
			LOG_DEBUG(("fsaa mode: %d", _fsaa));
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, _fsaa);
		}
	
		//_window.set_video_mode(w, h, 0,  SDL_OPENGL | SDL_OPENGLBLIT | flags );
#ifdef USE_GLSDL
		flags |= SDL_GLSDL;
#endif

		_window.set_video_mode(_w, _h, 0, flags );

#if SDL_VERSION_ATLEAST(1,2,10)

		int accel = -1;
		if ((r = SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &accel)) == 0) {
			LOG_DEBUG(("SDL_GL_ACCELERATED_VISUAL = %d", accel));

		
			if (!_force_soft && accel != 1) {
				throw_ex(("Looks like you don't have a graphics card that is good enough.\n"
				"Please ensure that your graphics card supports OpenGL and the latest drivers are installed.\n" 
				"Try --force-soft-gl switch to enable sofware GL renderer."
				"Or use --no-gl to switch disable GL renderer completely."
				));
			}
		} else LOG_WARN(("SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL) failed: %s, result: %d, value: %d", SDL_GetError(), r, accel));
#endif

#ifndef __APPLE__
		LOG_DEBUG(("vendor: %s", getGLString(GL_VENDOR).c_str()));
		LOG_DEBUG(("renderer: %s", getGLString(GL_RENDERER).c_str()));
#endif

	} else {
		_window.set_video_mode(_w, _h, 0, flags);
	}

#else //_WINDOWS

#ifdef USE_GLSDL
		flags |= _dx?SDL_GLSDL : 0;
#endif
		_window.set_video_mode(_w, _h, 0, flags);
	
#endif

	LOG_DEBUG(("created main surface. (%dx%dx%d, %s)", _w, _h, _window.getBPP(), ((_window.getFlags() & SDL_HWSURFACE) == SDL_HWSURFACE)?"hardware":"software"));

	sdlx::System::probe_video_mode();	

	_running = true;
}

void IWindow::run() {

	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 100);
	
	_fr = fps_limit;
	int max_delay = 1000000 / fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));

	SDL_Event event;
	while (_running) {
		_timer.reset();
		
		while (SDL_PollEvent(&event)) {
			event_signal.emit(event);
		
			switch(event.type) {
			case SDL_JOYBUTTONDOWN:
				joy_button_signal.emit(event.jbutton.which, event.jbutton.button, event.jbutton.type == SDL_JOYBUTTONDOWN);
			break;
			
			case SDL_KEYUP:			
			case SDL_KEYDOWN:
				key_signal.emit(event.key.keysym, event.type == SDL_KEYDOWN);
			break;
			
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mouse_signal.emit(event.button.button, event.button.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				break;
			
			case SDL_MOUSEMOTION:
				mouse_motion_signal.emit(event.motion.state, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
				break;
			
		    //case SDL_QUIT:
			//	_running = false;
			//break;
    		}
		}
		
		const float dt = 1.0/_fr;
		
		tick_signal.emit(dt);
		
		IWindow::flip();

		int t_delta = _timer.microdelta();

		//LOG_DEBUG(("tdelta: %d, max_delay: %d, delay: %d", t_delta, max_delay, max_delay - t_delta));
		assert(t_delta >= 0);
		if (t_delta < max_delay) {
			sdlx::Timer::microsleep("fps limit", max_delay - t_delta);
		}

		t_delta = _timer.microdelta();
		_fr = (t_delta != 0)? (1000000.0 / t_delta): 1000000;
	}
	LOG_DEBUG(("exiting main loop."));
	if (_running)
		throw_sdl(("SDL_WaitEvent"));

}

void IWindow::init_dummy() {
    LOG_DEBUG(("initializing dummy video driver..."));
	SDL_putenv(strdup("SDL_VIDEODRIVER=dummy"));
    sdlx::System::init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
	sdlx::Surface::set_default_flags(SDL_SRCALPHA);
    _window.set_video_mode(640, 480, 0, SDL_ANYFORMAT);
}

void IWindow::deinit() {
#ifdef _WINDOWS
	AllowAccessibilityShortcutKeys(true);
#endif
	_running = false;
	LOG_DEBUG(("shutting down, freeing surface"));
	_window.free();
}

IWindow::~IWindow() {
	//_window.free();
}

void IWindow::flip() {
	_window.flip();
}

void IWindow::resetTimer() {
	_timer.reset();
}
