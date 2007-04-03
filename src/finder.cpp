#include "finder.h"
#include "config.h"
#include "mrt/fs_node.h"
#include "mrt/fmt.h"

IMPLEMENT_SINGLETON(Finder, IFinder)

IFinder::IFinder() {
#ifdef PREFIX
	GET_CONFIG_VALUE("engine.path", std::string, path, RESOURCES_DIR "/private/data:" RESOURCES_DIR "/data");
#else
	GET_CONFIG_VALUE("engine.path", std::string, path, "private/data:data");
#endif
	std::vector<std::string> r;
	mrt::split(r, path, ":");
	for(size_t i = 0; i < r.size(); ++i) {
		if (mrt::FSNode::exists(r[i]))
			_path.push_back(r[i]);
		else LOG_DEBUG(("skipped non-existent path item %s", r[i].c_str()));
	}
	if (_path.empty())
		throw_ex(("non of the directories listed in engine.path('%s') exist", path.c_str()));
}

const std::string IFinder::find(const std::string &name, const bool strict) const {
	for(size_t i = 0; i < _path.size(); ++i) {
		std::string file = _path[i] + "/" + name;
		if (mrt::FSNode::exists(file))
			return file;
	}
	if (strict)
		throw_ex(("file '%s' not found", name.c_str()));
	return std::string();
}

void IFinder::findAll(FindResult &result, const std::string &name) const {
	result.clear();
	
	for(size_t i = 0; i < _path.size(); ++i) {
		std::string file = _path[i] + "/" + name;
		if (mrt::FSNode::exists(file))
			result.push_back(FindResult::value_type(_path[i], file));
	}
	if (result.empty())
		throw_ex(("file '%s' not found", name.c_str()));
}

void IFinder::getPath(std::vector<std::string> &path) const {
	path = _path;
}
