#ifndef MRT_ZIP_FILE_H__
#define MRT_ZIP_FILE_H__

#include "export_mrt.h"
#include <stdio.h>
#include "base_file.h"

namespace mrt {
class MRTAPI ZipFile : public mrt::BaseFile {
public: 
	ZipFile(FILE *file, const unsigned method, const unsigned flags, const unsigned offset, const unsigned csize, const unsigned usize);
	
	virtual void open(const std::string &fname, const std::string &mode);
	virtual bool opened() const;

	virtual void seek(long offset, int whence) const;
	virtual long tell() const;
	virtual void write(const mrt::Chunk &ch) const;

	virtual const off_t get_size() const;
	virtual const size_t read(void *buf, const size_t size) const;
	virtual void close();
	
	virtual bool eof() const;

	virtual ~ZipFile();

private: 
	FILE *file;
	const unsigned method, flags, offset;
	long csize, usize;
	mutable long voffset;
};

}

#endif

