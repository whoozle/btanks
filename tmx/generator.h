#ifndef BTANKS_GENERATOR_H__
#define BTANKS_GENERATOR_H__

#include <string>
#include <vector>
#include <map>
#include "sdlx/sdlx.h"
#include <stack>
#include "math/matrix.h"

class Layer;
class Tileset;
class GeneratorObject;

class MapGenerator {
public: 
	MapGenerator();
	~MapGenerator();

	void exec(Layer *layer, const std::string &command, const std::string &value);
	void tileset(const std::string &name, const int gid);
	void clear();

	//layer proxy
	const Uint32 get(const int x, const int y) const; 
	void set(const int x, const int y, const Uint32 tid);

private: 

	void fill(Layer *layer, const std::vector<std::string> &args);
	void fillPattern(Layer *layer, const std::vector<std::string> &args);
	void pushMatrix(Layer *layer, const std::vector<std::string> &args);
	void popMatrix(Layer *layer, const std::vector<std::string> &args);
	void exclude(Layer *layer, const std::vector<std::string> &args);
	
	const GeneratorObject *getObject(const std::string &tileset, const std::string &name) const;
	
	static const std::string getName(const std::string &fname);
	static const std::string getDescName(const std::string &fname);
	
	std::map<const std::string, int> first_gid;
	typedef std::map<const std::string, Tileset *> Tilesets;
	Tilesets _tilesets;
	
	Layer *_layer;
	typedef Matrix<int> IntMatrix; 
	std::stack<IntMatrix> _matrix_stack;
};

#endif
