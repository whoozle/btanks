#ifndef BTANKS_TILESET_H__
#define BTANKS_TILESET_H__

#include "mrt/xml.h"

class GeneratorObject;
class Tileset : public mrt::XMLParser {
public: 

	const GeneratorObject *getObject(const std::string &name) const;
	~Tileset();

private: 
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);

	Attrs  _attr;
	std::string _cdata;

	typedef std::map<const std::string, GeneratorObject *> Objects;
	Objects _objects;
};

#endif

