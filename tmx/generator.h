#ifndef BTANKS_GENERATOR_H__
#define BTANKS_GENERATOR_H__

#include <string>
#include <vector>
#include <map>

class Layer;
class Tileset;
class GeneratorObject;

class MapGenerator {
public: 
	MapGenerator();

	void exec(Layer *layer, const std::string &command, const std::string &value);
	void tileset(const std::string &name, const int gid);
	void clear();

private: 

	void fill(Layer *layer, const std::vector<std::string> &value);
	
	const GeneratorObject *getObject(const std::string &tileset, const std::string &name) const;
	
	static const std::string getName(const std::string &fname);
	static const std::string getDescName(const std::string &fname);
	
	std::map<const std::string, int> first_gid;
	typedef std::map<const std::string, Tileset *> Tilesets;
	Tilesets _tilesets;
};

#endif
