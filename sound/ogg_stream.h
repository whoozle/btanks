#ifndef __BTANKS_OGG_STREAM_H__
#define __BTANKS_OGG_STREAM_H__

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
class Sample;
class OggStream : public sdlx::Thread {
public: 
	void open(const std::string &fname, const bool continuous);
	void close();

	const bool alive() const { return _running; }	
		
	OggStream();
	~OggStream();
	
	static void decode(Sample &sample, const std::string &file);
	void setVolume(const float v);

private: 


	const bool playing() const;
	const bool play();
	virtual const int run(); 
	const bool update();
	void empty();
	void _open();
	const bool stream(ALuint buffer);

	std::string _filename;
	FILE * _file;
	OggVorbis_File _ogg_stream;
	vorbis_info * _vorbis_info;
	vorbis_comment * _vorbis_comment;

	ALuint _buffers_n, _buffers[32];
	ALuint _source;
	ALenum _format;
	
	volatile bool _opened, _running, _repeat;
	int _delay;
};

#endif

