#include "al_ex.h"
#include <assert.h>
#include <AL/alut.h>

ALException::ALException(const ALenum code): _code(code) {}
const std::string ALException::getCustomMessage() {
	const char * err = alutGetErrorString(_code);
	assert(err != NULL);
	return err;
}
