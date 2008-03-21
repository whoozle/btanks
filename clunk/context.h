#ifndef CLUNK_CONTEXT_H__
#define CLUNK_CONTEXT_H__

#include "export_clunk.h"
#include "object.h"
#include "sample.h"
#include <SDL_audio.h>
#include <map>
#include <set>
#include "mrt/chunk.h"

namespace clunk {

class Stream;

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
	
	void play(const int id, Stream *stream, bool loop);
	bool playing(const int id) const;
	void pause(const int id);
	void stop(const int id);
	void set_volume(const int id, float volume);

	void set_fx_volume(float volume);
	void stop_all(bool stop_streams);

private: 
	SDL_AudioSpec spec;
	int period_size;

	static void callback(void *userdata, Uint8 *stream, int len);
	void delete_object(Object *o);

	friend clunk::Object::~Object();
	friend clunk::Sample::~Sample();
	
	typedef std::set<Object *> objects_type;
	objects_type objects;
	
	struct stream_info {
		bool loop;
		Stream *stream;
		float gain;
		bool paused;
		mrt::Chunk buffer;
	};
	
	typedef std::map<const int, stream_info> streams_type;
	streams_type streams;

	v3<float> listener;
	unsigned max_sources;
};
}


#endif

