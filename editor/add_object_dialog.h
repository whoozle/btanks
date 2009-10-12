#ifndef BTANKS_EDITOR_ADD_OBJECT_DIALOG_H__
#define BTANKS_EDITOR_ADD_OBJECT_DIALOG_H__

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

#include "menu/scroll_list.h"
#include "mrt/xml.h"
#include <set>
#include <string>

class ObjectPropertiesDialog;

class AddObjectDialog : public ScrollList, private mrt::XMLParser {
public: 
	AddObjectDialog(const int w, const int h);
	~AddObjectDialog();
	virtual bool onKey(const SDL_keysym sym);
	virtual void tick(const float dt);
	
	const bool get(std::string &classname, std::string &animation, int &z);
	
	void get_variants(std::set<std::string> &variants, const std::string &classname) const;

private: 
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);

	// temp parser objects 
	std::string _classname;
	//end of parser objects

	std::string _base, _fname;
	
	typedef std::set<std::string> Classes; 
	typedef std::multimap<const std::string, std::string> Animations, Variants;
	typedef std::vector<Animations::const_iterator> UserData;
	
	Classes _classes; 
	Animations _animations;
	Variants _variants;
	UserData _user_data;
	
	std::string selected_classname, selected_animation;
	int selected_z;
	
	ObjectPropertiesDialog *_z;
};

#endif

