#include "mrt/exception.h"
#include <stdlib.h>
#include "clunk.h"

int main() {
	TRY {
		clunk::Context context;
		context.init(22050, 2, 1024);
		clunk::Sample * sample = context.create_sample();
		sample->generateSine(440, 1);
		
		clunk::Object * o = context.create_object();
		o->play(new clunk::Source(sample));
		sleep(2);
		delete o; o = NULL;
		
		context.deinit();
	} CATCH("main", return 1); 

	return 0;
}
