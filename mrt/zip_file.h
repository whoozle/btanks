#ifndef MRT_ZIP_FILE_H__
#define MRT_ZIP_FILE_H__

#include "export_mrt.h"
#include "base_file.h"

namespace mrt {
class MRTAPI ZipFile : public mrt::BaseFile {
public: 
	virtual void open(const std::string &fname, const std::string &mode);
	virtual const bool opened() const;

	virtual int seek(long offset, int whence) const;
	virtual long tell() const;
	virtual void write(const mrt::Chunk &ch) const;

	virtual const off_t getSize() const;
	virtual const size_t read(void *buf, const size_t size) const;
	virtual void close();
	
	virtual const bool eof() const;
};

}

#endif

