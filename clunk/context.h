#ifndef CLUNK_CONTEXT_H__
#define CLUNK_CONTEXT_H__

#include "export_clunk.h"
#include "object.h"
#include "sample.h"
#include <SDL_audio.h>

namespace clunk {
class CLUNKAPI Context {
public: 
	Context();
	
	void init(const int sample_rate, const Uint8 channels, int period_size);
	void deinit();
	
	Object *createObject();
	Sample *createSample();
	
	~Context();
private: 
	SDL_AudioSpec spec;
	int period_size;

	static void callback(void *userdata, Uint8 *stream, int len);
	void deleteObject(Object *o);

	friend clunk::Object::~Object();
	friend clunk::Sample::~Sample();
};
}


#endif

