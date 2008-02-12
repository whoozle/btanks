#ifndef ZZIP_MRT_FILE_H__
#define ZZIP_MRT_FILE_H__

#include "mrt/base_file.h"
#include <zzip/zzip.h>
#include "export_zzip.h"

namespace sdlx {
	class Mutex;
} 

namespace zzip {
	class ZZIPAPI File : public mrt::BaseFile {
	public: 
		//File();
		File(ZZIP_FILE *f, const sdlx::Mutex &big_lock);

		virtual void open(const std::string &fname, const std::string &mode);
		virtual const bool opened() const;
	
		virtual int seek(long offset, int whence) const;
		virtual long tell() const;
		virtual void write(const mrt::Chunk &ch) const;

		virtual const off_t getSize() const;
		virtual const size_t read(void *buf, const size_t size) const;
		virtual void close();
	
		virtual const bool eof() const;

		~File();
	private: 
		ZZIP_FILE *_f;
		mutable bool _endof;
		const sdlx::Mutex &big_lock;
	};
}

#endif
