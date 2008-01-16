#include "crash.h"

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#ifndef _WINDOWS
#	include <string.h>
#	include <signal.h>
#	include <unistd.h>
#	include <stdlib.h>
#	include <stdio.h>

static void crash_handler(int sno) {
	fprintf(stdout, "btanks crashed with signal %d. use gdb -p %d to debug it. zzZzzZZzz...\n\n", sno, getpid());
	sleep(3600);
}

#endif

void mrt::install_crash_handlers() {
#ifndef _WINDOWS
	if (getenv("MRT_NO_CRASH_HANDLER") != NULL)
		return;
	
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

}
