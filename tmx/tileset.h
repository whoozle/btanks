#ifndef BTANKS_TILESET_H__
#define BTANKS_TILESET_H__

#include "mrt/xml.h"

class Tileset : public mrt::XMLParser {
public: 
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
};

#endif

