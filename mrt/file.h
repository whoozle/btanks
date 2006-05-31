#ifndef __MRT_BASEFILE__
#define __MRT_BASEFILE__

#include <string>
#include <stdio.h>

namespace mrt {

class Chunk;

class File {
	public: 
	File();
	void open(const std::string &fname, const std::string &mode);
	void readAll(Chunk &ch) const;
	const std::string readLine() const;
	const off_t getSize() const;
	const size_t read(void *buf, const size_t size) const;
	void close();
	
	const bool eof() const;
	
	private: 
	FILE *_f;
};

}

#endif
