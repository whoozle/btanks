#ifndef MRT_ZIP_DIR_H__
#define MRT_ZIP_DIR_H__

#include "export_mrt.h"
#include "base_directory.h"
#include "file.h"

namespace mrt {
class ZipFile;

struct LocalZipHeader;

class MRTAPI ZipDirectory : public mrt::BaseDirectory {
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
	mrt::File archive;
};

}

#endif

