#include "ogg_stream.h"
#include "config.h"
#include "ogg_ex.h"
#include <assert.h>
#include "mrt/chunk.h"
#include "mrt/file.h"
#include "config.h"
#include "sample.h"
#include "al_ex.h"

OggStream::OggStream() : _opened(false) {
	GET_CONFIG_VALUE("engine.sound.polling-interval", int, delay, 10);
	_delay = delay;
}


void OggStream::open(const std::string &fname) {
	close();
	_filename = fname;
	_running = true;
	start();
}


void OggStream::_open(const std::string &fname) {
	mrt::File file;
	file.open(fname, "rb");
	int r = ov_open(file, &_ogg_stream, NULL, 0);
	if (r < 0)
		throw_ogg(r, ("ov_open('%s')", fname.c_str()));
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
	if(playing())
		return true;

	for(unsigned int i = 0; i <  _buffers_n; ++i) {	
		if(!stream(_buffers[0]))
			return false;
	}
	alSourceQueueBuffers(_source, _buffers_n, _buffers);
	alSourcePlay(_source);
	return false;
}

const bool OggStream::update() {
	int processed;
	bool active = true;

	alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);

	while(processed--) {
		ALuint buffer;
		alSourceUnqueueBuffers(_source, 1, &buffer);
		AL_CHECK(("alSourceUnqueueBuffers"));
		
		active = stream(buffer);
		alSourceQueueBuffers(_source, 1, &buffer);
		AL_CHECK(("alSourceQueueBuffers"));
	}
	return active;
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
	alDeleteBuffers(1, _buffers);
	AL_CHECK(("alDeleteBuffers"));					    

	LOG_DEBUG(("deleting ogg context."));
	ov_clear(&_ogg_stream);					   
	_opened = false;
}

OggStream::~OggStream() {
	close();
}

const bool OggStream::playing() const {
	ALenum state;
	alGetSourcei(_source, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}

const bool OggStream::stream(ALuint buffer) {
	mrt::Chunk data;
	
	GET_CONFIG_VALUE("engine.sound.file-buffer-size", int, buffer_size, 32768);
	data.setSize(buffer_size);
	
	int  size = 0;
	int  section;

	while(size < buffer_size) {
		int r = ov_read(&_ogg_stream, ((char *)data.getPtr()) + size, buffer_size - size, 0, 2, 1, & section);
    
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
	_open(_filename);
    while(_running && update()) {
		if(!playing()) {
			if(!play()) {
				LOG_ERROR(("Ogg abruptly stopped."));
			} else
				LOG_WARN(("ogg stream was interrupted.."));
		}
		SDL_Delay(_delay);
	}
	_running = false;
	LOG_DEBUG(("sound thread exits.."));
	return 0;
}
