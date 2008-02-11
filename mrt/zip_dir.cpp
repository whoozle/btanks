#include "zip_dir.h"

using namespace mrt;

//mrt::ZipDirectory xxx;

ZipDirectory::ZipDirectory() {}

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

ZipDirectory::~ZipDirectory() {}

ZipFile * ZipDirectory::open_file(const std::string &name) const {
	return NULL;
}
