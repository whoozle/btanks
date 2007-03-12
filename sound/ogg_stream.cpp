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

OggStream::OggStream() : _opened(false), _running(false), _repeat(false) {
	GET_CONFIG_VALUE("engine.sound.polling-interval", int, delay, 10);
	_delay = delay;
}


void OggStream::open(const std::string &fname, const bool repeat) {
	close();
	_filename = fname;
	_running = true;
	_repeat = repeat;
	start();
}


void OggStream::_open() {
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
		
	GET_CONFIG_VALUE("engine.sound.buffers", int, bf, 4);
	if (bf < 1 || bf > 32) 
		throw_ex(("engine.sound.buffers must be in (1,32) range (%d)", bf));
	_buffers_n = bf;
	alGenBuffers(bf, _buffers);
	AL_CHECK(("alGenBuffers(%d)", bf));
    alGenSources(1, &_source);
	AL_CHECK(("alGenSources(oggsource)"));

	alSource3f(_source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(_source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(_source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (_source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (_source, AL_SOURCE_RELATIVE, AL_TRUE      );
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
		alSourcePlay(_source);
		return true;
	}
} CATCH("play()", throw;)
	return false;
}

const bool OggStream::update() {
TRY {
	int processed = 0;
	bool active = true;

	alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
	AL_CHECK(("alGetSourcei(processed: %d)", processed));

	while(processed--) {
		ALuint buffer;
		alSourceUnqueueBuffers(_source, 1, &buffer);
		AL_CHECK(("alSourceUnqueueBuffers"));
		
		active = stream(buffer);
		if (!active) 
			break;
		alSourceQueueBuffers(_source, 1, &buffer);
		AL_CHECK(("alSourceQueueBuffers"));
	}
} CATCH("update()", throw;)
	return true;
}


void OggStream::empty() {
	int n = 0;
	alGetSourcei(_source, AL_BUFFERS_QUEUED, &n);
	while(n--) {
		ALuint buffer;
		alSourceUnqueueBuffers(_source, 1, &buffer);
		TRY {
			AL_CHECK(("alSourceUnqueueBuffers"));
		} CATCH("empty", {})
	}					
}

void OggStream::close() {
	if (!_opened)
		return;
	
	_running = false;
	wait();
	LOG_DEBUG(("deleting al source/buffers"));

	alSourceStop(_source);

	empty();

	alDeleteSources(1, &_source);
	AL_CHECK(("alDeleteSources"));
	alDeleteBuffers(_buffers_n, _buffers);
	AL_CHECK(("alDeleteBuffers"));					    

	LOG_DEBUG(("deleting ogg context."));
	ov_clear(&_ogg_stream);					   
	_opened = false;
}

OggStream::~OggStream() {
	close();
}

const bool OggStream::playing() const {
TRY {
	ALenum state;
	alGetSourcei(_source, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
} CATCH("playing()", throw; )
}

const bool OggStream::stream(ALuint buffer) {
TRY {
	if (!_opened)
		return false;
	
	mrt::Chunk data;
	
	GET_CONFIG_VALUE("engine.sound.file-buffer-size", int, buffer_size, 32768);
	data.setSize(buffer_size);
	
	int  size = 0;
	int  section;

	while(size < buffer_size) {
		int r = ov_read(&_ogg_stream, ((char *)data.getPtr()) + size, buffer_size - size, 0, 2, 1, & section);
		//LOG_DEBUG(("ov_read(%d) = %d (section: %d)", size, r, section));
    
		if(r > 0) {
			size += r;
		} else if(r < 0) {
			throw_ogg(r, ("ov_read"));
		} else break;
	}
	
	if(size == 0)
		return false;
 
	alBufferData(buffer, _format, data.getPtr(), size, _vorbis_info->rate);
	AL_CHECK(("alBufferData"));
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

const int OggStream::run() {
TRY {
	do {
		TRY {
			_open();
		} CATCH("run::_open", throw;)
	TRY {
    	while(_running && update()) {
			if(!playing()) {
				if(!play()) {
					LOG_WARN(("Ogg abruptly stopped."));
					break;
				} else
					LOG_WARN(("ogg stream was interrupted.."));
			} else 
				SDL_Delay(_delay);
		}
	} CATCH("run(main loop)", throw;)
	TRY { 
		while(_running) {
			ALenum state;
			alGetSourcei(_source, AL_SOURCE_STATE, &state);
	
			if (state != AL_PLAYING)
				break;
			else
				SDL_Delay(_delay);
		}
	} CATCH("run(flush)", throw;)
		
	} while(_running && _repeat);	
	_running = false;
	LOG_DEBUG(("sound thread exits.."));
	return 0;
} CATCH("OggStream::run", {})
return 1;
}
