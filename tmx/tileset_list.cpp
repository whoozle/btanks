#include "tileset_list.h"
#include "mrt/exception.h"

void TilesetList::clear() {
	_tilesets.clear();
	_last_gid = 0;
}

void TilesetList::add(const std::string &name, const int gid, const int size) {
	if (gid == 0)
		throw_ex(("adding tileset with gid 0 prohibited"));

	_tilesets.push_back(Tilesets::value_type(name, gid));
	if (gid + size - 1 > _last_gid)
		_last_gid = gid + size - 1;
}

const int TilesetList::exists(const std::string &name) const {
	size_t n = size();
	for(size_t i = 0; i < n; ++i) {
		if (_tilesets[i].first == name)
			return _tilesets[i].second;
	}
	return 0;
}
