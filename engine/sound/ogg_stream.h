#ifndef __BTANKS_OGG_STREAM_H__
#define __BTANKS_OGG_STREAM_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include "clunk/stream.h"

namespace mrt {
	class BaseFile;
}

namespace clunk {
	class Sample;
	class Buffer;
}

class OggStream : public clunk::Stream {
public: 
	static void decode(clunk::Sample &sample, const std::string &file);

	OggStream(const std::string &fname);
	void rewind();
	bool read(clunk::Buffer &data, unsigned hint);
	~OggStream();

private: 
	mrt::BaseFile * _file;
	OggVorbis_File _ogg_stream;
	vorbis_info * _vorbis_info;
};

#endif

