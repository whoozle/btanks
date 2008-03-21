/* libclunk - realtime 2d/3d sound render library
 * Copyright (C) 2005-2008 Netive Media Group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


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


	void convert(mrt::Chunk &dst, const mrt::Chunk &src, int rate, const Uint16 format, const Uint8 channels);

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
		stream_info() : stream(NULL), loop(false), gain(1.0f), paused(false), buffer() {}
		Stream *stream;
		bool loop;
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

