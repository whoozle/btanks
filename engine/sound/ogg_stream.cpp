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


#include "ogg_stream.h"
#include "config.h"
#include "ogg_ex.h"
#include <assert.h>
#include "sdlx/timer.h"
#include "scoped_ptr.h"
#include "finder.h"
#include "mrt/base_file.h"
#include "mrt/chunk.h"

OggStream::OggStream() : _idle(true), _volume(1) {}

void OggStream::play(const std::string &fname, const bool repeat, const float volume) {
	LOG_DEBUG(("play('%s', %s, %g)", fname.c_str(), repeat?"loop":"once", volume));
	stop();
	scoped_ptr<mrt::BaseFile> file(Finder->get_file(fname, "rb"));
}

void OggStream::stop() {
}

OggStream::~OggStream() {
}

#include "scoped_ptr.h"
#include <vorbis/vorbisfile.h>

static size_t stream_read_func  (void *ptr, size_t size, size_t nmemb, void *datasource) {
	//LOG_DEBUG(("read(%p, %u, %u)", ptr, (unsigned)size, (unsigned)nmemb));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		int r = file->read(ptr, nmemb * size);
		if (r <= 0)
			return r;
	
		return r / size;
	} CATCH("read_cb", return -1);
}

static int    stream_seek_func  (void *datasource, ogg_int64_t offset, int whence) {
	//LOG_DEBUG(("seek(%u, %d)", (unsigned)offset, whence));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		file->seek(offset, whence);
		return 0;
	} CATCH("seek_cb", return -1);
}

static int    stream_close_func (void *datasource) {
	//LOG_DEBUG(("close()"));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		file->close();
		return 0;
	} CATCH("close_cb", return -1);
}

static long   stream_tell_func  (void *datasource) {
	//LOG_DEBUG(("tell"));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		return file->tell();
	} CATCH("tell_cb", return -1);
}


void OggStream::decode(mrt::Chunk &data, const std::string &fname, int &rate, int &channels) {
	scoped_ptr<mrt::BaseFile> file(Finder->get_file(fname, "rb"));
	
	ov_callbacks ov_cb;
	memset(&ov_cb, 0, sizeof(ov_cb));

	ov_cb.read_func = stream_read_func;
	ov_cb.seek_func = stream_seek_func;
	ov_cb.tell_func = stream_tell_func;
	ov_cb.close_func = stream_close_func;
		
	OggVorbis_File ogg;
	int r = ov_open_callbacks(file.get(), &ogg, NULL, 0, ov_cb);
	if (r < 0)
		throw_ogg(r, ("ov_open('%s')", fname.c_str()));

	GET_CONFIG_VALUE("engine.sound.file-buffer-size", int, buffer_size, 441000);

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
	
	channels = info->channels;
	rate = info->rate;

	ov_clear(&ogg);	
}

void OggStream::setVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	
	_volume = volume;	
}
