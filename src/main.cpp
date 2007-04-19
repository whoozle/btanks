/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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
#include "game.h"
#include "version.h"
#include <stdlib.h>

DLLIMPORT void btanks_objects_dummy_exp_method(void);


#ifdef WIN32
#	include "sdlx/SDL_main.h"
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else 
#	include <signal.h>
#	include <unistd.h>

static void crash_handler(int sno) {
	fprintf(stdout, "btanks crashed with signal %d. use gdb -p %d to debug it. zzZzzZZzz...\n\n", sno, getpid());
	sleep(3600);
}


#endif

#ifdef __cplusplus
extern "C"
#endif
	int main(int argc, char *argv[]);


int main(int argc, char *argv[]) {
	try {
		LOG_NOTICE(("starting up... version: %s", getVersion().c_str()));
		btanks_objects_dummy_exp_method();
#ifndef WIN32
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = crash_handler;
		
		if (sigaction(SIGSEGV, &sa, NULL) == -1) 
			perror("sigaction");
		if (sigaction(SIGABRT, &sa, NULL) == -1) 
			perror("sigaction");
		if (sigaction(SIGFPE, &sa, NULL) == -1) 
			perror("sigaction");
		if (sigaction(SIGILL, &sa, NULL) == -1) 
			perror("sigaction");
		if (sigaction(SIGBUS, &sa, NULL) == -1) 
			perror("sigaction");

#endif		
		Game->init(argc, argv);
		Game->run();
		Game->deinit();
		LOG_DEBUG(("exiting"));
#ifdef WIN32
	} catch(const std::exception &e) {
		LOG_ERROR(("main:%s", e.what()));
		TRY { LOG_DEBUG(("calling Game->deinit()")); Game->deinit(); } CATCH("deinit", {});
		
		MessageBox(NULL, e.what(), "Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		return 1;
	}
#else 
	} CATCH("main", { 
		TRY { LOG_DEBUG(("calling Game->deinit()")); Game->deinit(); } CATCH("deinit", {});		
		return 1;
	})
#endif
	return 0;
}
