#include "crash.h"

#ifndef WIN32
#	include <string.h>
#	include <signal.h>
#	include <unistd.h>
#	include <stdlib.h>
#	include <stdio.h>
#endif

static void crash_handler(int sno) {
	fprintf(stdout, "btanks crashed with signal %d. use gdb -p %d to debug it. zzZzzZZzz...\n\n", sno, getpid());
	sleep(3600);
}

void mrt::install_crash_handlers() {
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

}
