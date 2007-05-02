/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "ogg_stream.h"
#include "config.h"
#include "ogg_ex.h"
#include <assert.h>
#include "mrt/chunk.h"
#include "mrt/file.h"
#include "config.h"
#include "sample.h"
#include "al_ex.h"

OggStream::OggStream(const ALuint source) : _source(source), _opened(false), _running(false), _repeat(false), _alive(true), _idle(false)  {
	GET_CONFIG_VALUE("engine.sound.polling-interval", int, delay, 10);
	_delay = delay;
	start();
}


void OggStream::play(const std::string &fname, const bool repeat, const float volume) {
	LOG_DEBUG(("play('%s', %s, %g)", fname.c_str(), repeat?"loop":"once", volume));
	sdlx::AutoMutex m(_lock);
	stop();
	_filename = fname;
	_repeat = repeat;

	_volume = volume;
	if (_idle) {
		_idle_sem.post();
		_running = true;
	} else {
		_running = false;
	}
}


void OggStream::_open() {
	sdlx::AutoMutex m(_lock);

	LOG_DEBUG(("_open()"));
	mrt::File file;
	file.open(_filename, "rb");
	int r = ov_open(file, &_ogg_stream, NULL, 0);
	if (r < 0)
		throw_ogg(r, ("ov_open('%s')", _filename.c_str()));
	_file = file.unlink();
	
	_vorbis_info = ov_info(&_ogg_stream, -1);
	_vorbis_comment = ov_comment(&_ogg_stream, -1);
	assert(_vorbis_info != NULL);
	
	if (_vorbis_info->channels == 1) 
		_format = AL_FORMAT_MONO16;
	else
		_format = AL_FORMAT_STEREO16;
	_opened = true;
	if (!_repeat)
		_filename.clear();
	
	GET_CONFIG_VALUE("engine.sound.buffers", int, bf, 8);
	if (bf < 1 || bf > 32) 
		throw_ex(("engine.sound.buffers must be in (1,32) range (%d)", bf));
	_buffers_n = bf;
	alGenBuffers(bf, _buffers);
	AL_CHECK(("alGenBuffers(%d)", bf));
	for(unsigned i = 0; i < _buffers_n; ++i) {
		if (alIsBuffer(_buffers[i]) == AL_FALSE) {
			if (i == 0) 
				throw_ex(("cannot generate %u buffers", _buffers_n));
			LOG_WARN(("buffer #%u is invalid. reducing buffers' counter to %u", i + 1, i));
			_buffers_n = i;
			break;
		}
	}

	alSource3f(_source, AL_POSITION,        0.0, 0.0, 0.0);
	AL_CHECK(("alSource3f(%08x, AL_POSITION, {0,0,0})", (unsigned)_source));
	alSource3f(_source, AL_VELOCITY,        0.0, 0.0, 0.0);
	AL_CHECK(("alSource3f(%08x, AL_VELOCITY, {0,0,0})", (unsigned)_source));
	alSource3f(_source, AL_DIRECTION,       0.0, 0.0, 0.0);
	AL_CHECK(("alSource3f(%08x, AL_DIRECTION, {0,0,0})", (unsigned)_source));
	alSourcef (_source, AL_ROLLOFF_FACTOR,  0.0          );
	AL_CHECK(("alSourcef(%08x, AL_ROLLOFF_FACTOR, 0.0)", (unsigned)_source));
	alSourcei (_source, AL_SOURCE_RELATIVE, AL_TRUE      );
	AL_CHECK(("alSourcei(%08x, AL_SOURCE_RELATIVE, AL_TRUE)", (unsigned)_source));
}

const bool OggStream::play() {
TRY {
	if(playing())
		return true;

	unsigned int i;
	for(i = 0; i < _buffers_n; ++i) {	
		if(!stream(_buffers[i]))
			break;
	}
	if (i > 0) {
		alSourceQueueBuffers(_source, i, _buffers);
		AL_CHECK(("alSourceQueueBuffers(%08x, %d, %p)", _source, i, (const void *)_buffers));
		alSourcePlay(_source);
		AL_CHECK(("alSourcePlay(%08x)", _source));
		return true;
	}
} CATCH("play()", throw;)
	return false;
}

const bool OggStream::update() {
if (!_running)
	return false;
TRY {
	int processed = 0;
	bool active = true;

	alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
	AL_CHECK(("alGetSourcei(processed: %d)", processed));
	/*
	if (processed != 0) {
		LOG_DEBUG(("source=%u, processed = %d", (unsigned)_source, processed));
		for(unsigned i = 0; i < _buffers_n; ++i) {
			LOG_DEBUG(("buffer[%d] = %u", i, _buffers[i]));
		}
	}
	*/
	int n = processed;
	while(n-- > 0) {
		ALuint buffer;
		alSourceUnqueueBuffers(_source, 1, &buffer);
		AL_CHECK(("alSourceUnqueueBuffers(%d of %d)", processed - n, processed));
		//LOG_DEBUG(("unqueued buffer: %u", (unsigned) buffer));
		
		TRY { 
			active = stream(buffer);
		} CATCH("update(stream)", throw;);
		//LOG_DEBUG(("stream returned %s", active?"true":"false"));
		if (!active) 
			continue;
		
		alSourceQueueBuffers(_source, 1, &buffer);
		//LOG_DEBUG(("queued buffer: %u", (unsigned) buffer));
		AL_CHECK(("alSourceQueueBuffers"));
	}
} CATCH("update()", throw;)
	return true;
}


void OggStream::empty() {
	int n = 0;
	alGetSourcei(_source, AL_BUFFERS_QUEUED, &n);
	AL_CHECK(("alGetSourcei(%08x, AL_BUFFERS_QUEUED)", _source));
	while(n--) {
		ALuint buffer;
		alSourceUnqueueBuffers(_source, 1, &buffer);
		AL_CHECK_NON_FATAL(("alSourceUnqueueBuffers(%08x, 1)", _source));
	}					
}

void OggStream::stop() {
	sdlx::AutoMutex m(_lock);
	_running = false;
	_filename.clear();
}

OggStream::~OggStream() {
	_alive = false;
	stop();
	if (_idle)
		_idle_sem.post();
	wait();
}

const bool OggStream::playing() const {
TRY {
	ALenum state;
	alGetSourcei(_source, AL_SOURCE_STATE, &state);
	AL_CHECK(("alGetSourcei(%08x, AL_SOURCE_STATE)", _source));
	return (state == AL_PLAYING);
} CATCH("playing()", throw; )
}

const bool OggStream::stream(ALuint buffer) {
TRY {
	if (!_opened)
		return false;
	
	mrt::Chunk data;
	
	GET_CONFIG_VALUE("engine.sound.file-buffer-size", int, buffer_size, 8192);
	data.setSize(buffer_size);
	
	int  size = 0;
	int  section;

	while(size < buffer_size) {
		int r = ov_read(&_ogg_stream, ((char *)data.getPtr()) + size, buffer_size - size, 0, 2, 1, & section);
		//LOG_DEBUG(("ov_read(%d) = %d (section: %d)", buffer_size - size, r, section));
    
		if(r > 0) {
			size += r;
		} else if(r < 0) {
			throw_ogg(r, ("ov_read"));
		} else break;
	}
	assert(size <= buffer_size);
	
	if(size == 0)
		return false;
 
	alBufferData(buffer, _format, data.getPtr(), size, _vorbis_info->rate);
	AL_CHECK(("alBufferData(size: %d, rate: %ld)", size, _vorbis_info->rate));
} CATCH("stream", throw;)
    return true;
}

void OggStream::decode(Sample &sample, const std::string &fname) {
	mrt::File file;
	file.open(fname, "rb");

	OggVorbis_File ogg;
	int r = ov_open(file, &ogg, NULL, 0);
	if (r < 0)
		throw_ogg(r, ("ov_open('%s')", fname.c_str()));
	file.unlink();

	GET_CONFIG_VALUE("engine.sound.file-buffer-size", int, buffer_size, 32768);

	mrt::Chunk &data = sample.data;

	size_t pos = 0;
	data.free();
	int section = 0;
	
	do {
		data.setSize(buffer_size + pos);
		r = ov_read(&ogg, ((char *)data.getPtr()) + pos, buffer_size, 0, 2, 1, & section);
		if (r == OV_HOLE) {
			LOG_WARN(("hole in ogg data, attempt to recover"));
			continue;
		}
    
		if(r > 0) {
			pos += r;
		} else if(r < 0) {
			ov_clear(&ogg);
			throw_ogg(r, ("ov_read"));
		} else break;
	} while(true);
	data.setSize(pos);
	
	vorbis_info *info = ov_info(&ogg, -1);
	assert(info != NULL);
	
	if (info->channels == 1) 
		sample.format = AL_FORMAT_MONO16;
	else
		sample.format = AL_FORMAT_STEREO16;
	sample.rate = info->rate;

	ov_clear(&ogg);	
}

void OggStream::playTune() {
	_running = true;
	TRY {
		_open();
	} CATCH("playTune::_open", throw;)

	TRY {
		setVolume(_volume);
	} CATCH("playTune::setVolume", )

	TRY {
    	while(_alive && _running && update()) {
			if(!playing()) {
				if(!play()) {
					LOG_WARN(("Ogg abruptly stopped."));
					break;
				} else
					LOG_WARN(("ogg stream was interrupted.."));
			} else 
				SDL_Delay(_delay);
		}
	} CATCH("playTune(main loop)", throw;)

	TRY { 
		while(_alive && _running) {
			ALenum state;
			alGetSourcei(_source, AL_SOURCE_STATE, &state);
			AL_CHECK(("alGetSourcei(%08x, AL_SOURCE_STATE)", (unsigned)_source));
	
			if (state != AL_PLAYING)
				break;
			else
				SDL_Delay(_delay);
		}
	} CATCH("playTune(flush)", throw;)

	LOG_DEBUG(("deleting al source/buffers"));

	alSourceStop(_source);
	AL_CHECK_NON_FATAL(("alSourceStop(%08x)", (unsigned)_source));

	TRY {
		empty();
	} CATCH("close", {});

	alDeleteBuffers(_buffers_n, _buffers);
	AL_CHECK_NON_FATAL(("alDeleteBuffers"));

	LOG_DEBUG(("deleting ogg context."));
	ov_clear(&_ogg_stream);					   
	_opened = false;
}

const int OggStream::run() {
TRY {
	while(_alive) {
		sdlx::AutoMutex m(_lock);

		if (_filename.empty()) {
			m.unlock();
			
			LOG_DEBUG(("sound thread idle..."));
			_idle = true;
			_idle_sem.wait();
			_idle = false;
			LOG_DEBUG(("sound thread woke up..."));
		} else m.unlock();
		
		if (!_alive)
			break;
		
		do {
			playTune();	
		} while(_running && _repeat);	
		_running = false;
	}
	return 0;
} CATCH("OggStream::run", { _alive = false; _running = false; })
return 1;
}

void OggStream::setVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	
	
	alSourcef(_source, AL_GAIN, volume);
	AL_CHECK(("alSourcef(AL_GAIN, %g)", volume));
}

