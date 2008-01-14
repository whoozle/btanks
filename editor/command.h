#ifndef BTANKS_EDITOR_COMMAND_H__
#define BTANKS_EDITOR_COMMAND_H__

#include <string>
#include <vector>
#include <set>
#include <map>
#include "utils.h"
#include "math/v2.h"
#include "variants.h"

class Layer;
class Object;
struct GameItem;

class Command {
	Layer *layer;
public: 
	enum ObjectCommandType {InvalidObject, CreateObject, MoveObject, DeleteObject, ChangeObjectProperties, RotateObject };
	//enum Type { LayerModification } type;
	explicit Command(Layer *layer); //layer modification 
	explicit Command(const std::string &classname, const std::string &animation, const v2<int> &position, const int z);
	explicit Command(const ObjectCommandType, const Object *object, const int arg = 0, const Variants & vars = Variants()); //object modification
	
	void setTile(const int x, const int y, const int tile);
	const int getTile(const int x, const int y);

	void save(const int x, const int y);
	void move(const int x, const int y);
	
	void exec();
	void undo();

	~Command();
	Object *getObject();
private: 
	void deleteObject(const int id);

	ObjectCommandType type;

	typedef std::vector< ternary<int, int, int> > Queue;
	Queue queue;
	
	typedef std::map<const std::pair<int, int>, int> Backup;
	Backup backup;
	
	std::string property_backup;
	std::string classname, animation;
	v2<int> position;	
	int z, z_backup;
	Variants vars, vars_backup;
};

#endif


