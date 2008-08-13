#ifndef BTANKS_EDITOR_COMMAND_H__
#define BTANKS_EDITOR_COMMAND_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

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


