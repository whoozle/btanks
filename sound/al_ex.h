#ifndef __BTASKS_SOUND_AL_EX_H__
#define __BTASKS_SOUND_AL_EX_H__

#include "mrt/exception.h"
#include <AL/al.h>

class ALException : public mrt::Exception { 
public:
	ALException(const ALenum code);
	const std::string getCustomMessage();
private: 
	ALenum _code;
}; 

#define throw_al(r, str) throw_generic_no_default(ALException, str, (r));

#define AL_CHECK(fmt) { ALenum r; \
	if ((r = alutGetError()) != AL_NO_ERROR) \
	throw_al(r, fmt); \
}

#endif

