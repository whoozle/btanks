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

#include "mrt/logger.h"
#include "mrt/random.h"
#include "sdlx/system.h"
#include "game.h"
#include <stdlib.h>

#ifdef _WINDOWS
#	include "sdlx/SDL_main.h"
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <signal.h>
#	include <stdio.h>

static void clean_exit(int sno) {
	Game->stop();
}

#endif

#ifdef __APPLE__
#	include <SDL.h>
#	include <SDLmain.h>
#else

#ifdef __cplusplus
extern "C"
#endif
	int main(int argc, char *argv[]);
#endif

int main(int argc, char *argv[]) {
	try {
#ifndef _WINDOWS
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_flags = SA_RESTART;
		sa.sa_handler = &clean_exit;

		if (sigaction(SIGTERM, &sa, NULL) == -1) 
			perror("sigaction");
		if (sigaction(SIGINT, &sa, NULL) == -1) 
			perror("sigaction");
		if (sigaction(SIGQUIT, &sa, NULL) == -1) 
			perror("sigaction");
#endif
		mrt::init_seed();
		
		Game->loadPlugins();

		Game->init(argc, argv);
		Game->run();
		Game->deinit();
		LOG_DEBUG(("exiting"));
#ifdef _WINDOWS
	} catch(const std::exception &e) {
		LOG_ERROR(("main:%s", e.what()));
		TRY { LOG_DEBUG(("calling Game->deinit()")); Game->deinit(); } CATCH("deinit", {});
		
		MessageBox(NULL, e.what(), "Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		sdlx::System::deinit();
		return 1;
	}
#else 
	} CATCH("main", { 
		TRY { LOG_DEBUG(("calling Game->deinit()")); Game->deinit(); } CATCH("deinit", {});		
		sdlx::System::deinit();
		return 1;
	})
#endif
	sdlx::System::deinit();
	return 0;
}

#include "mrt/directory.h"
#include <string>

#ifdef _WINDOWS
extern "C" {
#ifdef _WIN32_WCE
int WINAPI SDLWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw)
#else
int WINAPI SDLWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
#endif
{
	std::string logpath = mrt::format_string("SDL_LOG_PATH=%s", mrt::Directory::get_app_dir("Battle Tanks", "btanks").c_str());
	_putenv(_strdup(logpath.c_str()));
	return SDLWinMain(hInst, hPrev, szCmdLine, sw);
}
}
#endif
