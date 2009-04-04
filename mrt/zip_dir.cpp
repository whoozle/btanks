#include <string.h>
#include "zip_dir.h"
#include "logger.h"
#include "chunk.h"
#include "exception.h"
#include "ioexception.h"
#include "zip_file.h"

using namespace mrt;

//mrt::ZipDirectory xxx;
struct LocalFileHeader {

	unsigned version;
	unsigned flags;
	unsigned method;
	unsigned mtime, mdate;
	unsigned crc32;
	unsigned csize, usize;
	
	std::string fname;
	mrt::Chunk extra;

	size_t data_offset;
	
private: 
	unsigned fsize, esize;
	
protected: 
	void read0(const mrt::BaseFile &file) {
		file.readLE16(version);
		file.readLE16(flags);
		file.readLE16(method);

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
			extra.set_size(fsize);
			if (file.read(extra.get_ptr(), fsize) != fsize)
				throw_ex(("unexpected end of archive"));

			fname.assign((const char *)extra.get_ptr(), fsize);
		} else {
			fname.clear();
		}

		if (esize > 0) {
			extra.set_size(esize);
			if (file.read(extra.get_ptr(), esize) != esize)
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
			comment.set_size(comment_size);
			if (file.read(comment.get_ptr(), comment_size) != comment_size)
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
			comment.set_size(comment_size);
			if (file.read(comment.get_ptr(), comment_size) != comment_size) 
				throw_ex(("unexpected end of the archive"));
		} else comment.free();
	}
};


ZipDirectory::ZipDirectory(const std::string &zip) : fname(zip) {
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
			
			FileDesc &file = headers[lfh.fname];
			file.flags = lfh.flags;
			file.method = lfh.method;
			file.csize = lfh.csize;
			file.usize = lfh.usize;
			file.offset = lfh.data_offset;
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
	LOG_DEBUG(("loaded %u files.", (unsigned)headers.size()));
}

void ZipDirectory::open(const std::string &path_) {
}

bool ZipDirectory::opened() const {
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

ZipFile * ZipDirectory::open_file(const std::string &name_) const {
	std::string name = mrt::FSNode::normalize(name_);
	Headers::const_iterator i = headers.find(name);
	if (i == headers.end())
		return NULL;
	const FileDesc &file = i->second;

	FILE * f = fopen(fname.c_str(), "rb");
	if (f == NULL)
		throw_io(("fopen(%s)", fname.c_str()));	
	
	return new ZipFile(f, file.method, file.flags, file.offset, file.csize, file.usize);
}

bool mrt::ZipDirectory::lessnocase::operator()(const std::string& s1, const std::string& s2) const {
#ifdef _WINDOWS
		return _stricmp(s1.c_str(), s2.c_str()) < 0;
#else
		return strcasecmp(s1.c_str(), s2.c_str()) < 0;
#endif
}

bool ZipDirectory::exists(const std::string &fname_) const {
	std::string fname = mrt::FSNode::normalize(fname_);
	return headers.find(fname) != headers.end();
}

void ZipDirectory::enumerate(std::vector<std::string>&files, const std::string &root) const {
	if (root.empty()) {
		for(Headers::const_iterator i = headers.begin(); i != headers.end(); ++i) {
			files.push_back(i->first);
		}
	} else {
		for(Headers::const_iterator i = headers.begin(); i != headers.end(); ++i) {
			const std::string &fname = i->first;
			if (fname.compare(0, root.size(), root))
				continue;
			std::string file = fname.substr(root.size() + 1);
			if (!file.empty())
				files.push_back(file);
		}
	}
}
