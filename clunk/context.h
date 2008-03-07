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
	
	Object *create_object();
	Sample *create_sample();
	
	~Context();
	
	const SDL_AudioSpec & get_spec() const {
		return spec;
	}
	
	void process(Sint16 *stream, int len);

private: 
	SDL_AudioSpec spec;
	int period_size;

	static void callback(void *userdata, Uint8 *stream, int len);
	void delete_object(Object *o);

	friend clunk::Object::~Object();
	friend clunk::Sample::~Sample();
	
	typedef std::set<Object *> objects_type;
	objects_type objects;

	v3<float> listener;
	unsigned max_sources;
};
}


#endif

