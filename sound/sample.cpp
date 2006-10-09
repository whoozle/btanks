#include "sample.h"
#include <AL/al.h>
#include "mrt/exception.h"

#define AL_CHECK(fmt) if (alGetError() != AL_NO_ERROR) \
	throw_ex(fmt)

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
