#ifndef BTANKS_EDITOR_OPEN_MAP_DIALOG_H__
#define BTANKS_EDITOR_OPEN_MAP_DIALOG_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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


#include "menu/container.h"
#include "menu/box.h"
#include <map>

class Button;
class Chooser;
class Prompt;
class NumberControl;

class OpenMapDialog : public Container {
public: 
	OpenMapDialog();
	
	virtual void tick(const float dt);
	virtual bool onKey(const SDL_Keysym sym);
	
	void load();
	void getMap(std::string &dir, std::string &name) const;

private: 
	Chooser *c_base, *c_map;
	Button  *b_ok, *b_back, *b_new;
	Prompt  *p_name;
	NumberControl *n_width, *n_height;
	std::string base, map;
	std::multimap<const std::string, std::string> _maps;
	std::map<const std::string, Chooser *> _map_chooser;
};

#endif

