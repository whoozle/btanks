#ifndef BTANKS_GENERATOR_OBJECT_H__
#define BTANKS_GENERATOR_OBJECT_H__

#include <string>
#include <map>
#include "export_btanks.h"

class MapGenerator;

class BTANKSAPI GeneratorObject {
public: 
	int w, h;
	GeneratorObject();

	virtual void init(const std::map<const std::string, std::string>& attrs, const std::string &data);
	virtual void render(MapGenerator *layer, const int first_gid, const int x, const int y, const bool full) const = 0;
	virtual ~GeneratorObject() {}
	
	static GeneratorObject *create(const std::string &name, const std::map<const std::string, std::string>& attrs, const std::string &data);
protected: 
	static std::string get(const std::map<const std::string, std::string>& attrs, const std::string &name);
private: 
	static GeneratorObject *create(const std::string &name);
};

namespace generator {

class BTANKSAPI TileBox : public GeneratorObject {
public: 
	int split_w[3];
	int split_h[3];
	void init(const std::map<const std::string, std::string>& _attrs, const std::string &data);
	void render(MapGenerator *gen, const int first_gid, const int x, const int y, const bool full) const;
};
}

#endif
