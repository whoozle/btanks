#ifndef __BTANKS_OGG_STREAM_H__
#define __BTANKS_OGG_STREAM_H__

#include <string>
#include <AL/al.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#include "sdlx/thread.h"
namespace mrt {
class Chunk;
}

class OggStream : public sdlx::Thread {
public: 
	void open(const std::string &fname);
	void close();
	
	const bool playing() const;
	const bool play();
		
	OggStream();
	~OggStream();
	
	static void decode(mrt::Chunk &data, const std::string &file);

private: 
	virtual const int run(); 
	const bool update();
	void empty();
	void _open(const std::string &fname);
	const bool stream(ALuint buffer);

	std::string _filename;
	FILE * _file;
	OggVorbis_File _ogg_stream;
	vorbis_info * _vorbis_info;
	vorbis_comment * _vorbis_comment;

	ALuint _buffers_n, _buffers[16];
	ALuint _source;
	ALenum _format;
	
	bool _opened, _running;
	int _delay;
};

#endif

