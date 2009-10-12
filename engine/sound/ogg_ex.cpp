
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

#include "ogg_ex.h"
#include "vorbis/codec.h" 
#include "mrt/fmt.h"

const std::string OggException::get_custom_message() {
	switch(_r) {
	case OV_FALSE:
		return "Not true, or no data available";
	case OV_HOLE:
		return "Vorbisfile encoutered missing or corrupt data in the bitstream. Recovery is normally automatic and this return code is for informational purposes only.";
	case OV_EREAD:
		return "Read error while fetching compressed data for decode";
	case OV_EFAULT:
		return "Internal inconsistency in decode state. Continuing is likely not possible.";
	case OV_EIMPL:
		return "Feature not implemented";
	case OV_EINVAL:
		return "Either an invalid argument, or incompletely initialized argument passed to libvorbisfile call";
	case OV_ENOTVORBIS:
		return "The given file/data was not recognized as Ogg Vorbis data.";
	case OV_EBADHEADER:
		return "The file/data is apparently an Ogg Vorbis stream, but contains a corrupted or undecipherable header.";
	case OV_EVERSION:
		return "The bitstream format revision of the given stream is not supported.";
	case OV_EBADLINK:
		return "The given link exists in the Vorbis data stream, but is not decipherable due to garbacge or corruption.";
	case OV_ENOSEEK:
		return "The given stream is not seekable";
	default:
		return mrt::format_string("Unknown error: %d", _r);
	}
}
