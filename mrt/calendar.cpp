#include "calendar.h"
#include <time.h>

bool mrt::xmas() {
	time_t t;
	time(&t);
	struct tm * d = localtime(&t);
	if (d->tm_mon == 0) {
		//jan
		if (d->tm_mday <= 7) 
			return true;
	} else if (d->tm_mon == 11) {
		//dec
		//xmas eve: 24th of dec
		if (d->tm_mday >= 24) 
			return true;
	}
	return false;
}
