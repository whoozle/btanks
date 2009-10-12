/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include "ogg_stream.h"
#include "finder.h"
#include "clunk/sample.h"
#include <assert.h>
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/base_file.h"
#include "mrt/chunk.h"
#include "config.h"
#include "ogg_ex.h"

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


OggStream::OggStream(const std::string &fname) {
	_file = Finder->get_file(fname, "rb");

	ov_callbacks ov_cb;
	memset(&ov_cb, 0, sizeof(ov_cb));

	ov_cb.read_func = stream_read_func;
	ov_cb.seek_func = stream_seek_func;
	ov_cb.tell_func = stream_tell_func;
	ov_cb.close_func = stream_close_func;

	int r = ov_open_callbacks(_file, &_ogg_stream, NULL, 0, ov_cb);
	if (r < 0)
		throw_ogg(r, ("ov_open('%s')", fname.c_str()));
	
	_vorbis_info = ov_info(&_ogg_stream, -1);

	sample_rate = _vorbis_info->rate;
	//LOG_DEBUG(("open(%s) : %d", fname.c_str(), sample_rate));
	format = AUDIO_S16LSB;
	channels = _vorbis_info->channels;

	//_vorbis_comment = ov_comment(&_ogg_stream, -1);
	assert(_vorbis_info != NULL);
}

void OggStream::rewind() {
	LOG_DEBUG(("rewinding stream..."));
	int r = ov_raw_seek(&_ogg_stream, 0);
	if (r != 0)
		throw_ogg(r, ("ov_raw_seek"));
}

bool OggStream::read(clunk::Buffer &data, unsigned hint) {
	if (hint == 0) 
		hint = 44100;
	
	data.set_size(hint);

	int section = 0;
	int r = ov_read(&_ogg_stream, (char *)data.get_ptr(), hint, 0, 2, 1, & section);
	//LOG_DEBUG(("ov_read(%d) = %d (section: %d)", hint, r, section));
	
	if(r >= 0) {
		data.set_size(r);
		
		return r != 0;
	}

	throw_ogg(r, ("ov_read"));
	return false; //:(
}

OggStream::~OggStream() {

}

#include "mrt/scoped_ptr.h"

void OggStream::decode(clunk::Sample &sample, const std::string &fname) {
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

	clunk::Buffer data;

	size_t pos = 0;
	data.free();
	int section = 0;
	
	do {
		data.set_size(buffer_size + pos);
		r = ov_read(&ogg, ((char *)data.get_ptr()) + pos, buffer_size, 0, 2, 1, & section);
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
	data.set_size(pos);
	
	vorbis_info *info = ov_info(&ogg, -1);
	assert(info != NULL);
	
	sample.init(data, info->rate, AUDIO_S16LSB, info->channels);

	ov_clear(&ogg);	
}
