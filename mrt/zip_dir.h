#ifndef MRT_ZIP_DIR_H__
#define MRT_ZIP_DIR_H__

#include "export_mrt.h"
#include "base_directory.h"
#include "file.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace mrt {
class ZipFile;

class MRTAPI ZipDirectory : public mrt::BaseDirectory, public mrt::FSNode {

struct lessnocase {
	bool operator()(const std::string& s1, const std::string& s2) const;
};

public: 
	ZipDirectory(const std::string &zip);
	
	virtual void open(const std::string &path);
	virtual bool opened() const;
	virtual const std::string read() const;
	virtual void close();
	virtual void create(const std::string &path, const bool recurse = false);
	virtual ~ZipDirectory();
	ZipFile * open_file(const std::string &name) const;

	//FSNode: 
	virtual bool exists(const std::string &fname) const;

	void enumerate(std::vector<std::string>&files, const std::string &root) const;

private: 
	struct FileDesc {
		unsigned flags, method, offset, csize, usize;
		FileDesc() : flags(0), method(0), offset(0), csize(0), usize(0) {}
	};
	mrt::File archive;
	typedef std::map<const std::string, FileDesc, lessnocase> Headers;
	Headers headers;
	std::string fname;
};

}

#endif

