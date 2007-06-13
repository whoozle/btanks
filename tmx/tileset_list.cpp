#include "tileset_list.h"
#include "mrt/exception.h"

void TilesetList::clear() {
	_tilesets.clear();
	_last_gid = 0;
}

const int TilesetList::add(const std::string &name, const int gid, const int size) {
	if (gid == 0)
		throw_ex(("adding tileset with gid 0 is prohibited"));

	int egid = gid;
	if (egid <= _last_gid) {
		LOG_DEBUG(("fixing invalid gid %d (the lowest value is %d)", gid, _last_gid));
		egid = _last_gid + 1;
	}

	_tilesets.push_back(Tilesets::value_type(name, egid));
	if (egid + size - 1 > _last_gid)
		_last_gid = egid + size - 1;
	
	return egid;
}

const int TilesetList::exists(const std::string &name) const {
	size_t n = size();
	for(size_t i = 0; i < n; ++i) {
		if (_tilesets[i].first == name)
			return _tilesets[i].second;
	}
	return 0;
}
