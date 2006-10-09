#include "ogg_ex.h"
#include "vorbis/codec.h" 
#include "mrt/fmt.h"

const std::string OggException::getCustomMessage() {
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
		return mrt::formatString("Unknown error: %d", _r);
	}
}
