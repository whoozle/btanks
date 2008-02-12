#include "zip_dir.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "mrt/exception.h"

using namespace mrt;

//mrt::ZipDirectory xxx;
struct LocalFileHeader {
	unsigned version;
	unsigned flags;
	unsigned compression;
	unsigned mtime, mdate;
	unsigned crc32;
	unsigned csize, usize;
	std::string fname;
	mrt::Chunk extra;
		
	void read(const mrt::BaseFile &file) {
		file.readLE16(version);
		file.readLE16(flags);
		file.readLE16(compression);

		file.readLE16(mtime);
		file.readLE16(mdate);
		
		file.readLE32(crc32);
		file.readLE32(csize);
		file.readLE32(usize);

		LOG_DEBUG(("local file record, version: %d, flags: %04x, compression: %d, crc32: %08x, size: %u/%u", version, flags, compression, crc32, csize, usize));
		
		unsigned fsize, esize;
		file.readLE16(fsize);
		file.readLE16(esize);
		
		if (fsize > 0) {
			extra.setSize(fsize);
			if (file.read(extra.getPtr(), fsize) != fsize)
				throw_ex(("unexpected end of archive"));

			fname.assign((const char *)extra.getPtr(), fsize);
		} else {
			fname.clear();
		}

		if (esize > 0) {
			extra.setSize(esize);
			if (file.read(extra.getPtr(), esize) != esize)
				throw_ex(("unexpected end of archive"));
		} else {
			extra.free();
		}
		LOG_DEBUG(("file: \"%s\", extra data: %s", fname.c_str(), extra.dump().c_str()));
	}
};


struct FileHeader {
	void read(const mrt::BaseFile &file) {
	}
};

ZipDirectory::ZipDirectory(const std::string &zip) {
	LOG_DEBUG(("opening archive: %s", zip.c_str()));
	archive.open(zip, "rb");
	unsigned magic;
	archive.readLE32(magic);
	LOG_DEBUG(("magic: %08x", magic));
	if (magic != 0x04034b50)
		throw_ex(("archive must start with local file header. invalid magic: %08x", magic));
	LocalFileHeader lfh;
	lfh.read(archive);
}

void ZipDirectory::open(const std::string &path) {
	
}

const bool ZipDirectory::opened() const {
	return false;
}

const std::string ZipDirectory::read() const {
	return std::string();
}

void ZipDirectory::close() {}

void ZipDirectory::create(const std::string &path, const bool recurse) {}

ZipDirectory::~ZipDirectory() {
	archive.close();
}

ZipFile * ZipDirectory::open_file(const std::string &name) const {
	return NULL;
}
