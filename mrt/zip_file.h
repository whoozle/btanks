#ifndef MRT_ZIP_FILE_H__
#define MRT_ZIP_FILE_H__

#include "export_mrt.h"
#include "base_file.h"

namespace mrt {
class MRTAPI ZipFile : public mrt::BaseFile {
public: 
	ZipFile(FILE *file, const unsigned method, const unsigned flags, const unsigned offset, const unsigned csize, const unsigned usize);
	
	virtual void open(const std::string &fname, const std::string &mode);
	virtual const bool opened() const;

	virtual int seek(long offset, int whence) const;
	virtual long tell() const;
	virtual void write(const mrt::Chunk &ch) const;

	virtual const off_t getSize() const;
	virtual const size_t read(void *buf, const size_t size) const;
	virtual void close();
	
	virtual const bool eof() const;

	virtual const bool readLine(std::string &str, const size_t bufsize = 1024) const;
private: 
	FILE *file;
	const unsigned method, flags, offset, csize, usize;
	long voffset;
};

}

#endif

