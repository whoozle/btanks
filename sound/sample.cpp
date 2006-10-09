#include "sample.h"
#include <AL/al.h>
#include "mrt/exception.h"
#include "al_ex.h"

void Sample::init() {
	TRY {
		alGenBuffers(1, &buffer);
		AL_CHECK(("alGenBuffers"));
	
		alBufferData(buffer, format, data.getPtr(), data.getSize(), rate);
		AL_CHECK(("alBufferData"));
	} CATCH("init", throw;)
}

Sample::~Sample() {
	alDeleteBuffers(1, &buffer);
}
