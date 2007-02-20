#ifndef BTANKS_MAP_PICKER_H__
#define BTANKS_MAP_PICKER_H__

#include "container.h"
#include "sdlx/surface.h"
#include <vector>

class MapPicker : public Container {
public: 
	MapPicker(const int w, const int h);

private:
	void loadScreenshot();
	void scan(const std::string &dir);

	sdlx::Surface _screenshot;

	size_t _index;
	typedef std::vector<std::pair<std::string, std::string> > MapList;
	MapList _maps;
};

#endif

