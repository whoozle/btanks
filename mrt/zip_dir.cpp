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
	
private: 
	unsigned fsize, esize;
	size_t data_offset;
	
protected: 
	void read0(const mrt::BaseFile &file) {
		file.readLE16(version);
		file.readLE16(flags);
		file.readLE16(compression);

		file.readLE16(mtime);
		file.readLE16(mdate);
		
		file.readLE32(crc32);
		file.readLE32(csize);
		file.readLE32(usize);

		//LOG_DEBUG(("version: %d, flags: %04x, compression: %d, crc32: %08x, size: %u/%u", version, flags, compression, crc32, csize, usize));
		
		file.readLE16(fsize);
		file.readLE16(esize);
	}

	void readFE(const mrt::BaseFile &file) {
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
		data_offset = file.tell();
		//LOG_DEBUG(("file: \"%s\", extra data: %s, data offset: %u", fname.c_str(), extra.dump().c_str(), (unsigned)data_offset));
	}

public: 
	void read(const mrt::BaseFile &file) {
		read0(file);
		readFE(file);
		file.seek(csize, SEEK_CUR);
	}
};

struct CentralDirectorySignature : public LocalFileHeader {
	mrt::Chunk comment;
	
	unsigned disk_number;
	unsigned internal_attrs, external_attrs;
	int header_offset;
private: 
	unsigned comment_size;
public: 	
	void read(const mrt::BaseFile &file) {
		unsigned version_made;
		file.readLE16(version_made);
		//LOG_DEBUG(("central directory signature, made by version %d", version_made));
		read0(file);
		file.readLE16(comment_size);
		file.readLE16(disk_number);
		
		file.readLE16(internal_attrs);
		file.readLE32(external_attrs);
		file.readLE32(header_offset);
		
		readFE(file);
		
		if (comment_size > 0) {
			comment.setSize(comment_size);
			if (file.read(comment.getPtr(), comment_size) != comment_size)
				throw_ex(("unexpected end of the archive"));
		} else {
			comment.free();
		}
		//LOG_DEBUG(("comment: %s, header offset: %d", comment.dump().c_str(), header_offset));
	}	
};


struct EndOfCentralDirectorySignature {
	unsigned disk_number, central_disk_number, central_on_this_disk;
	unsigned entries;
	
	unsigned size;
	int central_offset;

	mrt::Chunk comment;
	
private: 
	unsigned comment_size;
public: 	
	void read(const mrt::BaseFile &file) {
		file.readLE16(disk_number);
		file.readLE16(central_disk_number);
		file.readLE16(central_on_this_disk);
		file.readLE16(entries);
		
		file.readLE32(size);
		file.readLE32(central_offset);
		file.readLE16(comment_size);
		if (comment_size > 0) {
			comment.setSize(comment_size);
			if (file.read(comment.getPtr(), comment_size) != comment_size) 
				throw_ex(("unexpected end of the archive"));
		} else comment.free();
	}
};


ZipDirectory::ZipDirectory(const std::string &zip) {
	LOG_DEBUG(("opening archive: %s", zip.c_str()));
	archive.open(zip, "rb");
	unsigned magic;
	while(!archive.eof()) {
		try {
			archive.readLE32(magic);
		} catch(...) {
			break;
		}
		if (magic == 0x04034b50) {
			LocalFileHeader lfh;
			lfh.read(archive);
		} else if (magic == 0x02014b50) {
			CentralDirectorySignature cds;
			cds.read(archive);
		} else if (magic == 0x06054b50) {
			EndOfCentralDirectorySignature ecds;
			ecds.read(archive);
		} else {
			LOG_WARN(("unknown magic: %08x", magic));
			break;
		}
	}
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
