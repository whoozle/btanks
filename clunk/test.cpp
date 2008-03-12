#define _USE_MATH_DEFINES
#include <math.h>
#include "mrt/exception.h"
#include <stdlib.h>
#include "clunk.h"

#ifdef _WINDOWS
#	include <Windows.h>
#	define usleep(x) Sleep( (x) / 1000 )
#endif

int main() {
	TRY {
		clunk::Context context;
		context.init(22050, 2, 1024);
		clunk::Sample * sample = context.create_sample();
		sample->generateSine(440, 1);
		
		clunk::Object * o = context.create_object();
		o->play(new clunk::Source(sample, true));
		
		for(int i = 0; i < 200; ++i) {
			o->update(clunk::v3<float>(2 * cos(i / 100.0 * M_PI * 2), 2 * sin(i / 100.0 * M_PI * 2), 0), clunk::v3<float>());
			usleep(10000);
		}
		o->update(clunk::v3<float>(), clunk::v3<float>());
		delete o; o = NULL;
		
		context.deinit();
	} CATCH("main", return 1); 

	return 0;
}
