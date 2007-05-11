#ifndef BTANKS_TILESET_LIST_H__
#define BTANKS_TILESET_LIST_H__

#include <vector>
#include <string>
#include "export_btanks.h"

class BTANKSAPI TilesetList {
	typedef std::vector<std::pair<std::string, int> > Tilesets;

public: 
	typedef Tilesets::value_type value_type;
	
	void clear();
	void add(const std::string &name, const int gid);
	const int exists(const std::string &name) const;

	const size_t size() const { return _tilesets.size(); }
	const value_type& operator[](const size_t i) const { return _tilesets[i]; }

private: 
	Tilesets _tilesets;
};


#endif

