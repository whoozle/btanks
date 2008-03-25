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


#include <SDL.h>
#include <SDL_audio.h>
#include "context.h"
#include <string.h>
#include "sdl_ex.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "source.h"
#include <assert.h>
#include <math.h>
#include <map>
#include <algorithm>
#include "locker.h"
#include "stream.h"
#include "object.h"

using namespace clunk;

Context::Context() : period_size(0), listener(NULL), max_sources(8), fx_volume(1) {
}

void Context::callback(void *userdata, Uint8 *bstream, int len) {
	Context *self = (Context *)userdata;
	assert(self != NULL);
	Sint16 *stream = (Sint16*)bstream;
	TRY {
		self->process(stream, len);
	} CATCH("callback", )
}

#include "mrt/timespy.h"

void Context::process(Sint16 *stream, int size) {
	//TIMESPY(("total"));

	v3<float> listener = this->listener->position;
	{
		//TIMESPY(("sorting objects"));
		std::sort(objects.begin(), objects.end(), Object::DistanceOrder(listener));
	}
	//LOG_DEBUG(("sorted %u objects", (unsigned)objects.size()));
	
	std::vector<std::pair<v3<float>, Source *> > lsources;
	int n = size / 2 / spec.channels;
	
	for(objects_type::iterator i = objects.begin(); i != objects.end(); ) {
		Object *o = *i;
		Object::Sources & sset = o->sources;
		if (sset.empty() && o->dead) {
			//autodeleted object
			delete o;
			i = objects.erase(i);
			continue;
		}
		for(Object::Sources::iterator j = sset.begin(); j != sset.end(); ) {
			Source *s = j->second;
			if (!s->playing()) {
				//LOG_DEBUG(("purging inactive source %s", j->first.c_str()));
				delete j->second;
				sset.erase(j++);
				continue;
			}
			if (lsources.size() < max_sources) {
				lsources.push_back(std::pair<v3<float>, Source *>(o->position + s->delta_position - listener, s));
			} else {
				s->update_position(n);
			}
			++j;
		}
		++i;
	}

	memset(stream, 0, size);

	for(streams_type::iterator i = streams.begin(); i != streams.end();) {
		//LOG_DEBUG(("processing stream %d", i->first));
		stream_info &stream_info = i->second;
		while ((int)stream_info.buffer.getSize() < size) {
			mrt::Chunk data;
			bool eos = !stream_info.stream->read(data, size);
			if (!data.empty() && stream_info.stream->sample_rate != spec.freq) {
				//LOG_DEBUG(("converting audio data from %u to %u", stream_info.stream->sample_rate, spec.freq));
				convert(data, data, stream_info.stream->sample_rate, stream_info.stream->format, stream_info.stream->channels);
			}
			stream_info.buffer.append(data);
			//LOG_DEBUG(("read %u bytes", (unsigned)data.getSize()));
			if (eos) {
				if (stream_info.loop) {
					stream_info.stream->rewind();
				} else {
					break;
				}
			}
		}
		int buf_size = stream_info.buffer.getSize();
		//LOG_DEBUG(("buffered %d bytes", buf_size));
		if (buf_size == 0) {
			//all data buffered. continue;
			LOG_DEBUG(("stream %d finished. dropping.", i->first));
			TRY {
				delete stream_info.stream;
			} CATCH("mixing stream", );
			streams.erase(i++);
			continue;
		}
		
		if (buf_size >= size)
			buf_size = size;

		int sdl_v = (int)floor(SDL_MIX_MAXVOLUME * stream_info.gain + 0.5f);
		SDL_MixAudio((Uint8 *)stream, (Uint8 *)stream_info.buffer.getPtr(), buf_size, sdl_v);
		
		if ((int)stream_info.buffer.getSize() > size) {
			memmove(stream_info.buffer.getPtr(), ((Uint8 *)stream_info.buffer.getPtr()) + size, stream_info.buffer.getSize() - size);
			stream_info.buffer.setSize(stream_info.buffer.getSize() - size);
		} else {
			stream_info.buffer.free();
		}
		
		++i;
	}
	
	mrt::Chunk buf;
	buf.setSize(size);
	
	//TIMESPY(("mixing sources"));
	//LOG_DEBUG(("mixing %u sources", (unsigned)lsources.size()));
	for(unsigned i = 0; i < lsources.size(); ++i ) {
		v3<float> & position = lsources[i].first;
		Source * source = lsources[i].second;
		float volume = source->process(buf, spec.channels, position, fx_volume);
		int sdl_v = (int)floor(SDL_MIX_MAXVOLUME * volume + 0.5f);
		//LOG_DEBUG(("%u: mixing source with volume %g (%d), distance^2: %g", i, volume, sdl_v, position.quick_length()));
		if (volume <= 0)
			continue;
		if (sdl_v == 0)
			continue;
		SDL_MixAudio((Uint8 *)stream, (Uint8 *)buf.getPtr(), size, sdl_v);
	}
}


Object *Context::create_object() {
	AudioLocker l;
	Object *o = new Object(this);
	objects.push_back(o);
	return o;
}

Sample *Context::create_sample() {
	AudioLocker l;
	return new Sample(this);
}


void Context::init(const int sample_rate, const Uint8 channels, int period_size) {
	SDL_AudioSpec src;
	memset(&src, 0, sizeof(src));
	src.freq = sample_rate;
	src.channels = channels;
	src.format = AUDIO_S16LSB;
	src.samples = period_size;
	src.callback = &Context::callback;
	src.userdata = (void *) this;
	
	this->period_size = period_size;
	
	if ( SDL_OpenAudio(&src, &spec) < 0 )
		throw_sdl(("SDL_OpenAudio(%d, %u, %d)", sample_rate, channels, period_size));
	if (src.format != AUDIO_S16LSB)
		throw_ex(("SDL_OpenAudio(%d, %u, %d) returned format %d", sample_rate, channels, period_size, spec.format));
	LOG_DEBUG(("opened audio device, sample rate: %d, period: %d", spec.freq, spec.samples));
//	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_PauseAudio(0);
	
	listener = create_object();
}

void Context::delete_object(Object *o) {
	AudioLocker l;
	objects_type::iterator i = std::find(objects.begin(), objects.end(), o);
	while(i != objects.end() && *i == o)
		i = objects.erase(i); //just for fun
}

void Context::deinit() {
	delete listener;
	listener = NULL;
	
	SDL_PauseAudio(1);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
	
Context::~Context() {
	deinit();
}


//MUSIC MIXER: 

void Context::play(const int id, Stream *stream, bool loop) {
	LOG_DEBUG(("play(%d, %p, %s)", id, (const void *)stream, loop?"'loop'":"'once'"));
	AudioLocker l;
	stream_info & stream_info = streams[id];
	delete stream_info.stream;
	stream_info.stream = stream;
	stream_info.loop = loop;
	stream_info.paused = false;
	stream_info.gain = 1.0f;
}

bool Context::playing(const int id) const {
	AudioLocker l;
	return streams.find(id) != streams.end();
}

void Context::pause(const int id) {
	AudioLocker l;
	streams_type::iterator i = streams.find(id);
	if (i == streams.end())
		return;
	
	i->second.paused = !i->second.paused;
}

void Context::stop(const int id) {
	AudioLocker l;
	streams_type::iterator i = streams.find(id);
	if (i == streams.end())
		return;
	
	TRY {
		delete i->second.stream;
	} CATCH(mrt::formatString("stop(%d)", id).c_str(), {
		streams.erase(i);
		throw;
	})
	streams.erase(i);
}

void Context::set_volume(const int id, float volume) {
	if (volume < 0)
		volume = 0;
	if (volume > 1)
		volume = 1;
		
	streams_type::iterator i = streams.find(id);
	if (i == streams.end())
		return;
	i->second.gain = volume;
}

void Context::set_fx_volume(float volume) {
	//LOG_WARN(("ignoring set_fx_volume(%g)", volume));
	if (volume  < 0)	
		fx_volume = 0;
	else if (volume > 1)
		fx_volume = 1;
	else 
		fx_volume = volume;
}

void Context::stop_all() {
	AudioLocker l;
	for(streams_type::iterator i = streams.begin(); i != streams.end(); ++i) {
		delete i->second.stream;
	}
	streams.clear();
}

void Context::convert(mrt::Chunk &dst, const mrt::Chunk &src, int rate, const Uint16 format, const Uint8 channels) {
	SDL_AudioCVT cvt;
	memset(&cvt, 0, sizeof(cvt));
	if (SDL_BuildAudioCVT(&cvt, format, channels, rate, spec.format, channels, spec.freq) == -1) {
		throw_sdl(("DL_BuildAudioCVT(%d, %04x, %u)", rate, format, channels));
	}
	size_t buf_size = (size_t)(src.getSize() * cvt.len_mult);
	cvt.buf = (Uint8 *)malloc(buf_size);
	cvt.len = src.getSize();

	assert(buf_size >= src.getSize());
	memcpy(cvt.buf, src.getPtr(), src.getSize());

	if (SDL_ConvertAudio(&cvt) == -1) 
		throw_sdl(("SDL_ConvertAudio"));

	dst.setData(cvt.buf, (size_t)(cvt.len * cvt.len_ratio), true);
}
