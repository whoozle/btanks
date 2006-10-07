#ifndef __BTANKS_OGG_STREAM_H__
#define __BTANKS_OGG_STREAM_H__

#include <string>
#include "mrt/file.h"
#include <AL/al.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#include "sdlx/thread.h"

class OggStream : public sdlx::Thread {
public: 
	void open(const std::string &fname);
	void empty();
	void close();
	
	const bool playing() const;
	virtual const int run(); 
	const bool play();
	const bool update();
		
	OggStream();
	~OggStream();
private: 
	void _open(const std::string &fname);
	const bool stream(ALuint buffer);

	std::string _filename;
	mrt::File _file;
	OggVorbis_File _ogg_stream;
	vorbis_info * _vorbis_info;
	vorbis_comment * _vorbis_comment;

	ALuint _buffers_n, _buffers[16];
	ALuint _source;
	ALenum _format;
	
	bool _opened;
	int _delay;
};

#endif

