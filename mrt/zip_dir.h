#ifndef MRT_ZIP_DIR_H__
#define MRT_ZIP_DIR_H__

#include "export_mrt.h"
#include "base_directory.h"
#include "file.h"
#include <map>
#include <set>
#include <string>

namespace mrt {
class ZipFile;

class MRTAPI ZipDirectory : public mrt::BaseDirectory {

struct lessnocase {
	bool operator()(const std::string& s1, const std::string& s2) const;
};

public: 
	ZipDirectory(const std::string &zip);
	
	virtual void open(const std::string &path);
	virtual const bool opened() const;
	virtual const std::string read() const;
	virtual void close();
	virtual void create(const std::string &path, const bool recurse = false);
	virtual ~ZipDirectory();
	ZipFile * open_file(const std::string &name) const;
private: 
	struct FileDesc {
		unsigned flags, method, offset, csize;
		FileDesc() : flags(0), method(0), offset(0), csize(0) {}
	};
	mrt::File archive;
	std::map<const std::string, FileDesc, lessnocase> headers;
	std::set<std::string, lessnocase> filenames;
};

}

#endif

