#ifndef BTANKS_TILESET_H__
#define BTANKS_TILESET_H__

#include "xml_parser.h"
#include <deque>
#include <string>
#include <map>

class GeneratorObject;
class Tileset : public XMLParser {
public: 

	const GeneratorObject *getObject(const std::string &name) const;
	void getPrimaryBoxes(std::deque<std::string> &boxes);
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

